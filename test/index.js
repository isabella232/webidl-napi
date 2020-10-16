'use strict';
const { spawnSync } = require('child_process');
const { readdirSync, lstatSync } = require('fs');
const path = require('path');

readdirSync(__dirname).forEach((item) => {
  const testDir = path.join(__dirname, item);
  if (lstatSync(testDir).isDirectory()) {
    spawnSync(process.execPath, [path.join(testDir, 'test.js')], {
      stdio: 'inherit'
    });
  }
});
