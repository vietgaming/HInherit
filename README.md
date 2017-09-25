# HInherit
Implementation of leaking game handles through forced inheritance

## How it works
Inject into whitelisted system process with PROCESS_ALL_ACCESS handles to game processes. Set the handle as inheritable, and call CreateProcessAsUser to spawn a session 1 usermode process that inherits that handle. Then execute all game cheat functionality from that usermode process.
