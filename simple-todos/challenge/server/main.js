import { Tasks } from '../imports/api/tasks.js';
import { Flag } from '../imports/api/flag.js';

DEFAULT_TASKS = [
  'Find the Beatnik culture',
  'Unique San Francisco architecture',
  "The World's Fair 1939",
  'Take a Bart',
  'Have a fortune cookie',
  'Investigate Karl the fog',
  'Earthquake??',
];

if(!Tasks.findOne()) {
  console.log("Inserting default tasks");

  DEFAULT_TASKS.forEach((t) => {
    Tasks.insert({
      text: t,
      createdAt: new Date(),
      username: 'Guide',
    });
  });
}

if(!Flag.findOne()) {
  Flag.insert({
    flag: 'CTF{meteor_js_does_san_francisco}',
  });
}
