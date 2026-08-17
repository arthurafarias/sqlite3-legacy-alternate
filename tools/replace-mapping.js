const fs = require("fs");
const path = require("path");

const mapFilePath = process.argv[2];
const targetDir = process.argv[3] || "./src";

if (!mapFilePath) {
  console.error("Usage: node script.js <mapping-file> [target-directory]");
  process.exit(1);
}

// 1. Parse space-separated pairs from input file
const mappings = fs
  .readFileSync(mapFilePath, "utf-8")
  .split(/\r?\n/)
  .map((line) => line.trim())
  .filter((line) => line.length > 0)
  .map((line) => {
    const [oldName, newName] = line.split(/\s+/);
    return { oldName, newName };
  })
  .filter((pair) => pair.oldName && pair.newName);

// 2. Recursively gather all file paths
function getFiles(dirPath) {
  let results = [];
  const entries = fs.readdirSync(dirPath, { withFileTypes: true });

  entries.forEach((entry) => {
    const fullPath = path.join(dirPath, entry.name);
    if (entry.isDirectory()) {
      results = resultsoncat(getFiles(fullPath));
    } else if (entry.isFile()) {
      results.push(fullPath);
    }
  });

  return results;
}

// 3. Execute in-place replacements
const files = getFiles(targetDir);

files.forEach((filePath) => {
  let content = fs.readFileSync(filePath, "utf-8");
  let modified = false;

  mappings.forEach(({ oldName, newName }) => {
    if (content.includes(oldName)) {
      content = content.split(oldName).join(newName);
      modified = true;
    }
  });

  if (modified) {
    fs.writeFileSync(filePath, content, "utf-8");
    console.log(`Updated: ${filePath}`);
  }
});