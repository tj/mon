
1.2.2 / 2013-06-10 
==================

 * fix waiting for child to exit on graceful shutdown

1.2.1 / 2013-05-31 
==================

 * fix: only invoke error command when specified
 * update commander

1.2.0 / 2012-12-17 
==================

  * add passing of PID to the error scripts

1.1.2 / 2012-12-13 
==================

  * update commander

1.1.1 / 2012-11-12 
==================

  * change SIGKILL to sig

1.1.0 / 2012-11-09 
==================

  * add `--on-error <cmd>`
  * add `--on-restart <cmd>`
  * add `--attempts <max>`
  * add restart limiting / bailing logic
  * change signalling of child to pgid to fix `dash` support

1.0.1 / 2012-11-07 
==================

  * fix child exit, reset signals
  * change graceful_exit to kill() with the same sig

1.0.0 / 2012-11-07 
==================

  * add --on-error <cmd>
  * remove pid log
  * update commander

0.0.3 / 2012-06-15 
==================

  * Refactor with commander

0.0.2 / 2012-05-02 
==================

  * Added: error when an unrecognized flag is passed
  * Added -m, --mon-pidfile. Closes #4
  * Added ms dep

