'use strict';

const fs = require('fs');
const path = require('path');
const { spawn } = require('child_process');

const cfg = require('./config');

const RCON_HOST = '127.0.0.1';

function copyLatestBuiltDll() {
	const candidateDirs = fs.readdirSync(cfg.REPO_ROOT, { withFileTypes: true })
		.filter((e) => e.isDirectory() && /^build/i.test(e.name))
		.map((e) => path.join(cfg.REPO_ROOT, e.name));

	let best = null;

	for (const dir of candidateDirs) {
		for (const rel of ['qwprogs.dll', path.join('Release', 'qwprogs.dll')]) {
			const p = path.join(dir, rel);

			if (fs.existsSync(p)) {
				const mtimeMs = fs.statSync(p).mtimeMs;

				if (!best || mtimeMs > best.mtimeMs) {
					best = { path: p, mtimeMs };
				}
			}
		}
	}

	if (!best) {
		throw new Error('No built qwprogs.dll found under ktx/build*/. Build the project first.');
	}

	fs.copyFileSync(best.path, path.join(cfg.KTX_DIR, 'qwprogs.dll'));
}

function clearQconsoleLogs(port) {
	for (const name of fs.readdirSync(cfg.SANDBOX_DIR)) {
		if (name.startsWith(`qconsole_${port}_`)) {
			fs.unlinkSync(path.join(cfg.SANDBOX_DIR, name));
		}
	}
}

// Manages one mvdsv.exe process for the duration of a single scenario.
// Deliberately re-copies qwprogs.dll and re-execs a fresh process per
// scenario rather than reusing a server across scenarios, so each scenario
// starts from known-clean state and an unexpected process exit can always
// be attributed to that scenario.
class Server {
	constructor({ port, map }) {
		this.port = port;
		this.map = map;
		this.rconPassword = cfg.readRconPassword();
		this.child = null;
		this.exitInfo = null; // set once the process exits, whether we asked or not
		this.requestedStop = false;
	}

	start() {
		copyLatestBuiltDll();
		clearQconsoleLogs(this.port);

		const args = [
			'-game', 'ktx',
			'-port', String(this.port),
			'+exec', cfg.SMOKETEST_CFG_NAME,
			'+logfile',
			'+map', this.map,
		];

		this.child = spawn(cfg.MVDSV_EXE, args, {
			cwd: cfg.SANDBOX_DIR,
			stdio: ['ignore', 'ignore', 'ignore'],
		});

		this.child.on('exit', (code, signal) => {
			this.exitInfo = { code, signal, requested: this.requestedStop };
		});
	}

	isAlive() {
		return this.child !== null && this.exitInfo === null;
	}

	// True if the process died without us asking it to -- the crash signal
	// the whole harness is built around (see plan Part 3.5).
	diedUnexpectedly() {
		return this.exitInfo !== null && !this.exitInfo.requested;
	}

	rcon(command, timeoutMs) {
		return require('./rcon').send(RCON_HOST, this.port, this.rconPassword, command, timeoutMs);
	}

	waitForStatus(opts) {
		return require('./rcon').waitForStatus(RCON_HOST, this.port, this.rconPassword, opts);
	}

	qconsoleLogPath() {
		const name = fs.readdirSync(cfg.SANDBOX_DIR)
			.find((n) => n.startsWith(`qconsole_${this.port}_`));

		return name ? path.join(cfg.SANDBOX_DIR, name) : null;
	}

	async stop() {
		if (!this.isAlive()) {
			return;
		}

		this.requestedStop = true;
		this.child.kill();

		const deadline = Date.now() + 5000;

		while (this.exitInfo === null && Date.now() < deadline) {
			await new Promise((r) => setTimeout(r, 100));
		}

		if (this.exitInfo === null) {
			// Didn't die gracefully; force it so it can't linger and hold the port.
			try {
				process.kill(this.child.pid, 'SIGKILL');
			} catch {
				// already gone
			}
		}
	}
}

module.exports = { Server };
