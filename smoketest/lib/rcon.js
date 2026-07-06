'use strict';

// Minimal QuakeWorld rcon client: connectionless (OOB) UDP packets of the
// form "\xff\xff\xff\xffrcon <password> <command>". Replies come back as a
// single OOB packet ("\xff\xff\xff\xffn<text>\0"); we strip the 5-byte
// header/type-char prefix and trailing NUL before returning.
//
// Empirically verified against a real mvdsv.exe: this reply channel *does*
// carry engine-printed text (e.g. the "status" command's player list), but
// NOT anything the ktx-lite mod itself prints via G_Printf/G_cprint/
// G_sprint (see plan Part 3.5) -- so callers should only rely on replies to
// native engine commands like "status", not "mod ...".

const dgram = require('dgram');

const OOB_HEADER = Buffer.from([0xff, 0xff, 0xff, 0xff]);

function send(host, port, password, command, timeoutMs = 2000) {
	return new Promise((resolve, reject) => {
		const socket = dgram.createSocket('udp4');
		const payload = Buffer.concat([
			OOB_HEADER,
			Buffer.from(`rcon ${password} ${command}\n`, 'ascii'),
		]);

		let settled = false;

		const timer = setTimeout(() => {
			if (settled) {
				return;
			}

			settled = true;
			socket.close();
			resolve(null); // timeout: caller decides what that means
		}, timeoutMs);

		socket.on('message', (msg) => {
			if (settled) {
				return;
			}

			settled = true;
			clearTimeout(timer);
			socket.close();

			let text = msg.toString('latin1');

			if (text.startsWith('\xff\xff\xff\xff')) {
				text = text.slice(5); // strip OOB header + reply type char
			}

			resolve(text.replace(/\0+$/, ''));
		});

		socket.on('error', (err) => {
			if (settled) {
				return;
			}

			settled = true;
			clearTimeout(timer);
			socket.close();
			reject(err);
		});

		socket.send(payload, port, host, (err) => {
			if (err && !settled) {
				settled = true;
				clearTimeout(timer);
				socket.close();
				reject(err);
			}
		});
	});
}

// Polls "status" until it gets a reply (server up + rcon password accepted)
// or the timeout elapses. Returns the last status text, or null on timeout.
async function waitForStatus(host, port, password, { timeoutMs = 15000, intervalMs = 500 } = {}) {
	const deadline = Date.now() + timeoutMs;

	for (;;) {
		const reply = await send(host, port, password, 'status', 1000).catch(() => null);

		if (reply !== null && !reply.startsWith('Bad rcon_password')) {
			return reply;
		}

		if (Date.now() >= deadline) {
			return null;
		}

		await new Promise((r) => setTimeout(r, intervalMs));
	}
}

// Returns the non-empty lines of a "status" reply that come after the
// "------" table divider -- i.e. whatever is printed about connected
// players/bots. The exact column layout differs between bots and real
// clients, so this deliberately doesn't try to parse structured fields;
// callers just need "is anyone/anything connected" and "did the count of
// connected entries change", which a line count answers well enough.
function statusConnectedLines(statusText) {
	const lines = statusText.split('\n');
	const dividerIdx = lines.findIndex((l) => l.startsWith('---'));

	if (dividerIdx === -1) {
		return [];
	}

	return lines.slice(dividerIdx + 1).map((l) => l.trim()).filter(Boolean);
}

module.exports = { send, waitForStatus, statusConnectedLines };
