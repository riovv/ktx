'use strict';

const fs = require('fs');
const path = require('path');

const ROOT = path.join(__dirname, '..');
const REPO_ROOT = path.join(ROOT, '..');
const SANDBOX_DIR = path.join(ROOT, 'sandbox');
const KTX_DIR = path.join(SANDBOX_DIR, 'ktx');
const ID1_DIR = path.join(SANDBOX_DIR, 'id1');
const MVDSV_EXE = path.join(SANDBOX_DIR, 'mvdsv.exe');
const RCON_PASSWORD_FILE = path.join(SANDBOX_DIR, '.rcon_password');
const RESULTS_DIR = path.join(ROOT, 'results');
const SMOKETEST_CFG_NAME = 'smoketest.cfg';

function readRconPassword() {
	if (!fs.existsSync(RCON_PASSWORD_FILE)) {
		throw new Error(`${RCON_PASSWORD_FILE} not found — run "node setup.js" first.`);
	}
	return fs.readFileSync(RCON_PASSWORD_FILE, 'utf8').trim();
}

module.exports = {
	ROOT,
	REPO_ROOT,
	SANDBOX_DIR,
	KTX_DIR,
	ID1_DIR,
	MVDSV_EXE,
	RCON_PASSWORD_FILE,
	RESULTS_DIR,
	SMOKETEST_CFG_NAME,
	readRconPassword,
};
