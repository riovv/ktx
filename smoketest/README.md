# ktx-lite smoketest

Headless, bot-driven runtime smoketest for ktx-lite. Boots a real MVDSV
server with the freshly built `qwprogs.dll`, drives it entirely over rcon,
has frogbots play a real match (or run for a fixed duration) in every mode
ktx-lite keeps, and fails if the server stops responding.

Unlike upstream CI (which only compiles the mod), this actually *runs* it —
the safety net for the ongoing trim work that keeps deleting subsystems.

Windows-only for now. Requires a Steam install of Quake (for `PAK0.PAK` /
`PAK1.PAK`) and a built `qwprogs.dll` under `../build*/` (see the repo's main
build instructions).

## Usage

```
node setup.js              # one-time sandbox assembly (idempotent)
node setup.js --force      # re-download MVDSV + regenerate the rcon password

node run.js                # run every scenario in scenarios.json
node run.js --scenario duel-dm4     # run just one
node run.js --keep-server           # on a FAILED scenario, leave mvdsv.exe
                                     # running (rcon password in
                                     # sandbox/.rcon_password) for live
                                     # inspection instead of killing it
```

Exit code is non-zero if any scenario failed. Per-run output (a copy of that
scenario's `qconsole_<port>_*.log` plus a `verdict.json`) is saved under
`results/<timestamp>/<scenario>/`.

`run.js` always re-copies the newest `qwprogs.dll` from `../build*/` before
each scenario, so you don't need to re-run `setup.js` after every rebuild —
just `run.js`.

## How a scenario runs

1. Start `mvdsv.exe -game ktx -port 27600 +exec smoketest.cfg +logfile +map <map>`.
2. Wait for rcon `status` to reply (server ready).
3. Fire each of the scenario's `setup` rcon lines in order (mode cfgs,
   cvars), then reload the map so they take effect at spawn.
4. `mod addbot <skill> [force]` once per bot (see Part 1 of the plan this
   was built from for the `mod` console-command dispatcher).
5. Watch: for `"match"`/`"timed"` scenarios, poll rcon `status` for the
   scenario's duration; N consecutive timeouts is a FAIL.
6. Map-change smoke check: `rcon map dm2`, then wait for `status` to reply
   again — exercises the ShutDown/respawn path, a classic crash site when
   deleting code.
7. Teardown (unless `--keep-server` and the scenario failed).

### Why "server stopped responding to rcon", not "process crashed"

Verified empirically against the real binary: a fatal mod-level error
(`G_Error`) does **not** exit the mvdsv process — it leaves it running but
permanently unresponsive to rcon. So responsiveness, not process exit, is
what the harness actually watches for (see `lib/watch.js`). Process exit is
still checked too, as a secondary signal for a genuine crash on some other
build.

The `errortest` scenario is a permanent self-test: it deliberately triggers
`mod errortest` (which calls `G_Error`) and asserts the harness correctly
detects the server going unresponsive. If `errortest` ever fails, the
detection mechanism itself is broken — trust nothing else in the run until
that's fixed.

The `server.isAlive()` (process-exit) path was verified separately with a
manual breakage drill: a one-line `*(volatile int *)0 = 1;` dropped into
`FrogbotsAddbot()`, rebuilt, correctly turned `duel-dm4` red with "process
exited unexpectedly" — confirming a genuine crash (not just a `G_Error`
hang) is also caught, then reverted.

## Adding a scenario

Add an entry to `scenarios.json`:

```json
{
  "name": "my-scenario",
  "map": "dm4",
  "setup": ["exec configs/usermodes/1on1/default.cfg", "k_mode 1", "timelimit 1"],
  "bots": { "count": 2, "skill": 10, "force": false },
  "mode": "match",
  "timelimitMinutes": 1
}
```

- `mode: "match"` — waits `(timelimitMinutes * 60) + 60` seconds. Use for
  scenarios where `timelimit`/`fraglimit` naturally end the match.
- `mode: "timed"` — waits `durationSec` seconds instead, no timelimit
  assumed. Use for matchless/bloodfest-style continuous modes.
- `mode: "errortest"` — special-cased in `run.js`; only the one built-in
  self-test scenario should use this.
- `bots.force: true` skips `FrogbotsCheckMapSupport()`. Currently set on
  *every* scenario, not just the SP maps -- see the bot-routing finding
  below.
- As ktx-lite trims modes, delete the corresponding entry here; as it keeps
  a new mode, add one. That's the whole change surface for the matrix.

## Known finding: bot-routing never detects support in this sandbox

`mod addbot` without `force` failed with `"Map <x> not supported for bots"`
on every map tried (dm3, dm4), even after 10+ real seconds of uptime with
`deathmatch` confirmed `1` the whole time -- so `bots.force: true` is set on
every scenario in `scenarios.json`, not only the SP maps. Two things were
run down and fixed/documented along the way, but the underlying "why does
`LoadBotRoutingFromFile()` never succeed here" is still open:

- **Fixed**: `FrogbotsCheckMapSupport()`'s backing flag (`map_supported` in
  `src/bot_loadmap.c`) is set by a *one-shot* check, exactly 20 engine
  frames after process start (`BotStartFrame()`'s static `bot_framecount`
  in `src/bot_commands.c` -- never re-armed, so it's once per *process*,
  not once per map load). It only sets the flag if `deathmatch` is already
  nonzero at that instant, and `deathmatch` otherwise only gets set later
  by the mod's own config-reset cascade (game-logic-triggered, so strictly
  after frame 0) -- a race a cold headless boot can easily lose. `setup.js`
  now sets `deathmatch 1` directly in `smoketest.cfg`, which applies at
  engine command-line parse time, before frame 1.
- **Documented, not fixed**: separately, any server with zero human
  players connected gets its map forced back to `k_defmap` (`dm3`) by a
  watchdog (`Spawn_DefMapChecker`, `src/world.c` -- a 0.5s timeout on a
  server's very first-ever map load, 60-90s otherwise). Since this harness
  never has a human client, that watchdog *always* eventually fires,
  regardless of what map you asked for. It doesn't explain the
  `map_supported` failure by itself (the fix above should make the very
  first, one-shot check land correctly on whatever map was requested at
  boot, before the watchdog reassigns anything) but it's a real interaction
  worth knowing about if bot-routing is revisited: rcon `map <x>` reloads
  issued *after* boot can never re-trigger the one-shot check, so they
  can't fix a map that was unsupported at frame 20, and the watchdog means
  the server won't even stay on the map you asked for anyway.
- Since dumb (unrouted) bots are already an accepted tradeoff for the SP
  maps per the original plan, applying `force` everywhere was the pragmatic
  call rather than sinking more time into the file-open/parse path itself.

## Phase 2 (not implemented)

`lib/server.js`'s process spawn/monitor/kill handling is written to be
reusable for a future scenario type that spawns a real ezQuake client
(`+connect localhost:27600` plus a scripted cfg) instead of only
console-added bots.
