#!/usr/bin/env node
'use strict';

const fs = require('fs');
const path = require('path');

const cfg = require('./lib/config');
const { Server } = require('./lib/server');
const { pollResponsive, waitForBroken } = require('./lib/watch');

const PORT = 27600;
const READY_TIMEOUT_MS = 15000;

const args = process.argv.slice(2);
const scenarioFilterIdx = args.indexOf('--scenario');
const scenarioFilter = scenarioFilterIdx !== -1 ? args[scenarioFilterIdx + 1] : null;
const keepServer = args.includes('--keep-server');

const allScenarios = JSON.parse(fs.readFileSync(path.join(__dirname, 'scenarios.json'), 'utf8'));
const scenarios = scenarioFilter
	? allScenarios.filter((s) => s.name === scenarioFilter)
	: allScenarios;

if (scenarioFilter && scenarios.length === 0) {
	console.error(`No scenario named "${scenarioFilter}" in scenarios.json`);
	process.exit(1);
}

function log(scenarioName, msg) {
	console.log(`[${scenarioName}] ${msg}`);
}

async function addBots(server, scenarioName, bots) {
	if (!bots) {
		return;
	}

	const { count, skill = 10, force = false } = bots;

	for (let i = 0; i < count; ++i) {
		const cmd = ['mod', 'addbot', String(skill), force ? 'force' : ''].filter(Boolean).join(' ');

		log(scenarioName, `rcon: ${cmd}`);

		const reply = await server.rcon(cmd, 2000).catch((e) => `ERR:${e.message}`);

		if (reply) {
			log(scenarioName, `  reply: ${JSON.stringify(reply)}`);
		}
	}
}

async function runScenario(scenario, runDir) {
	const resultDir = path.join(runDir, scenario.name);

	fs.mkdirSync(resultDir, { recursive: true });

	const server = new Server({ port: PORT, map: scenario.map });
	let verdict = null;

	try {
		log(scenario.name, `starting mvdsv on map ${scenario.map}...`);
		server.start();

		let status = await server.waitForStatus({ timeoutMs: READY_TIMEOUT_MS });

		if (status === null) {
			verdict = { pass: false, reason: 'server did not become ready after start' };

			return verdict;
		}

		for (const line of scenario.setup || []) {
			log(scenario.name, `rcon: ${line}`);
			await server.rcon(line);
		}

		if (scenario.setup && scenario.setup.length) {
			// Re-load the map so mode cvars set above take effect at spawn,
			// same as a human admin would do after changing mode cvars.
			await server.rcon(`map ${scenario.map}`);
			status = await server.waitForStatus({ timeoutMs: READY_TIMEOUT_MS });

			if (status === null) {
				verdict = { pass: false, reason: 'server did not come back after mode-setup map reload' };

				return verdict;
			}
		}

		if (scenario.mode === 'errortest') {
			log(scenario.name, 'rcon: mod errortest (expecting the server to stop responding)');
			await server.rcon('mod errortest', 500).catch(() => null); // reply may never arrive

			const broke = await waitForBroken(server, 10000);

			verdict = broke
				? { pass: true }
				: { pass: false, reason: 'mod errortest did not break the server -- red-path detection is broken' };

			return verdict;
		}

		await addBots(server, scenario.name, scenario.bots);

		const durationSec = scenario.durationSec
			|| (scenario.timelimitMinutes ? (scenario.timelimitMinutes * 60) + 60 : 90);

		log(scenario.name, `watching for ${durationSec}s...`);

		const health = await pollResponsive(server, { durationMs: durationSec * 1000 });

		if (!health.healthy) {
			verdict = { pass: false, reason: health.reason };

			return verdict;
		}

		log(scenario.name, 'map-change smoke check (rcon map dm2)...');
		await server.rcon('map dm2');
		status = await server.waitForStatus({ timeoutMs: READY_TIMEOUT_MS });

		if (status === null) {
			verdict = { pass: false, reason: 'server did not survive the map-change smoke check' };

			return verdict;
		}

		verdict = { pass: true };

		return verdict;
	} catch (err) {
		verdict = { pass: false, reason: `harness error: ${err.message}` };

		return verdict;
	} finally {
		const logPath = server.qconsoleLogPath();

		if (logPath && fs.existsSync(logPath)) {
			fs.copyFileSync(logPath, path.join(resultDir, 'qconsole.log'));
		}

		fs.writeFileSync(
			path.join(resultDir, 'verdict.json'),
			JSON.stringify({ scenario: scenario.name, ...verdict }, null, 2));

		if (!keepServer || (verdict && verdict.pass)) {
			await server.stop();
		} else {
			log(scenario.name, `--keep-server: leaving mvdsv.exe (pid ${server.child.pid}) running for inspection`);
		}
	}
}

async function main() {
	const runDir = path.join(cfg.RESULTS_DIR, new Date().toISOString().replace(/[:.]/g, '-'));

	fs.mkdirSync(runDir, { recursive: true });

	const results = [];

	for (const scenario of scenarios) {
		const verdict = await runScenario(scenario, runDir);

		results.push({ name: scenario.name, ...verdict });
		log(scenario.name, verdict.pass ? 'PASS' : `FAIL: ${verdict.reason}`);
	}

	console.log('\n--- summary ---');

	let anyFailed = false;

	for (const r of results) {
		console.log(`${r.pass ? 'PASS' : 'FAIL'}  ${r.name}${r.pass ? '' : `  (${r.reason})`}`);
		anyFailed = anyFailed || !r.pass;
	}

	console.log(`\nresults: ${runDir}`);
	process.exit(anyFailed ? 1 : 0);
}

main().catch((err) => {
	console.error('[run] FAILED:', err);
	process.exit(1);
});
