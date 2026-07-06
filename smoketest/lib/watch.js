'use strict';

// The empirical crash signal for this build (see plan Part 3.5): triggering
// a fatal mod-level error (G_Error/trap_Error) does NOT exit the mvdsv
// process -- verified live, the process stays in the OS process list
// (child.exitCode stays null) but stops answering rcon entirely and never
// recovers. So "is the scenario healthy" means "does rcon status still get
// a reply", not "is the process still alive" -- the latter is checked too,
// as a secondary signal for a genuine crash/segfault, but responsiveness is
// what actually catches G_Error.

async function pollResponsive(server, { durationMs, intervalMs = 2000, failThreshold = 3 }) {
	const deadline = Date.now() + durationMs;
	let consecutiveFailures = 0;

	while (Date.now() < deadline) {
		if (!server.isAlive()) {
			return { healthy: false, reason: 'process exited unexpectedly' };
		}

		const reply = await server.rcon('status', 2000).catch(() => null);

		if (reply === null) {
			consecutiveFailures += 1;

			if (consecutiveFailures >= failThreshold) {
				return { healthy: false, reason: 'server stopped responding to rcon' };
			}
		} else {
			consecutiveFailures = 0;
		}

		const wait = Math.min(intervalMs, Math.max(0, deadline - Date.now()));

		await new Promise((r) => setTimeout(r, wait));
	}

	return { healthy: true };
}

// For the errortest self-test: we *want* to see the server go unresponsive
// (or, on some other build, actually exit) soon after "mod errortest".
// Resolves true once that's observed, false if it keeps responding normally
// for the whole timeout (meaning the red-path isn't actually being caught).
async function waitForBroken(server, timeoutMs, { intervalMs = 1000, failThreshold = 2 } = {}) {
	const deadline = Date.now() + timeoutMs;
	let consecutiveFailures = 0;

	while (Date.now() < deadline) {
		if (!server.isAlive()) {
			return true;
		}

		const reply = await server.rcon('status', 1500).catch(() => null);

		if (reply === null) {
			consecutiveFailures += 1;

			if (consecutiveFailures >= failThreshold) {
				return true;
			}
		} else {
			consecutiveFailures = 0;
		}

		await new Promise((r) => setTimeout(r, intervalMs));
	}

	return false;
}

module.exports = { pollResponsive, waitForBroken };
