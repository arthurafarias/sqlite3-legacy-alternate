const fs = require("fs");
const path = require("path");

const targetDir = process.argv[2] || "./src";

function toSnakeCase(str) {
  return str
    .replace(/([a-z0-9])([A-Z])/g, "$1_$2") // camelCase / PascalCase to snake_case
    .replace(/[\s\W]+/g, "_")              // spaces & special characters to _
    .replace(/^_+|_+$/g, "")               // trim leading/trailing _
    .toLowerCase();
}

function renameFiles(dirPath) {
  const entries = fs.readdirSync(dirPath, { withFileTypes: true });

  entries.forEach((entry) => {
    const fullPath = path.join(dirPath, entry.name);

    if (entry.isDirectory()) {
      renameFiles(fullPath); // Recursively handle subfolders
    } else if (entry.isFile()) {
      const ext = path.extname(entry.name);
      const baseName = path.basename(entry.name, ext);

      const newBaseName = toSnakeCase(baseName);
      const newFileName = `${newBaseName}${ext.toLowerCase()}`;
      const newFullPath = path.join(dirPath, newFileName);

      if (fullPath !== newFullPath) {
        fs.renameSync(fullPath, newFullPath);
        console.log(`Renamed: ${entry.name} -> ${newFileName}`);
      }
    }
  });
}

renameFiles(targetDir);