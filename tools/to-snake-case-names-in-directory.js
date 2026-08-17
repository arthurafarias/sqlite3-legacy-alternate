const fs = require("fs");
const path = require("path");

const listFilePath = process.argv[2];
const targetDir = process.argv[3] || "./src";

if (!listFilePath) {
  console.error("Usage: node script.js <path-to-list-file> [target-directory]");
  process.exit(1);
}

function toSnakeCase(str) {
  return str
    .replace(/([a-z0-9])([A-Z])/g, "$1_$2")
    .replace(/[\s\W]+/g, "_")
    .replace(/^_+|_+$/g, "")
    .toLowerCase();
}

function getFiles(dirPath) {
  let results = [];
  const entries = fs.readdirSync(dirPath, { withFileTypes: true });

  entries.forEach((entry) => {
    const fullPath = path.join(dirPath, entry.name);
    if (entry.isDirectory()) {
      results = results.concat(getFiles(fullPath));
    } else if (entry.isFile()) {
      results.push(fullPath);
    }
  });

  return results;
}

// 1. Read names from file
const rawList = fs.readFileSync(listFilePath, "utf-8");
const names = rawList
  .split(/\r?\n/)
  .map((line) => line.trim())
  .filter((line) => line.length > 0);

// 2. Scan and update files in directory
const files = getFiles(targetDir);

files.forEach((filePath) => {
  let content = fs.readFileSync(filePath, "utf-8");
  let modified = false;

  names.forEach((oldName) => {
    const newName = toSnakeCase(oldName);
    if (oldName !== newName && content.includes(oldName)) {
      content = content.split(oldName).join(newName);
      modified = true;
    }
  });

  if (modified) {
    fs.writeFileSync(filePath, content, "utf-8");
    console.log(`Updated: ${filePath}`);
  }
});