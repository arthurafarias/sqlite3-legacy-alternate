let fs = require("fs");
let path = require("path");

let args = process.argv;

// 1. Convert file contents from a Buffer to array of lines
let rawData = fs.readFileSync(args[2], "utf-8");
let class_list = rawData.split(/\r?\n/).filter((line) => line.trim() !== "");

// Utility function to convert PascalCase/camelCase to snake_case
function toSnake(str) {
    return str
        .replace(/([a-z0-9])([A-Z])/g, "$1_$2")
        .toLowerCase();
}

// Helper to recursively collect all file paths in a directory
function getFiles(dirPath) {
    let results = [];
    let list = fs.readdirSync(dirPath);
    list.forEach((file) => {
        let filePath = path.join(dirPath, file);
        let stat = fs.statSync(filePath);
        if (stat && stat.isDirectory()) {
            results = results.concat(getFiles(filePath));
        } else {
            results.push(filePath);
        }
    });
    return results;
}

let srcDir = path.join(fs.realpathSync(args[3]));
let allFiles = getFiles(srcDir);

// 2. Process each old name from the file
class_list.forEach((oldName) => {
    // 3. Fix variable shadowing (renamed parameter to oldName)
    let newName = toSnake(oldName);

    // Perform replacement across all files in src/
    allFiles.forEach((filePath) => {
        let content = fs.readFileSync(filePath, "utf-8");
        if (content.includes(oldName)) {
            let updatedContent = content.split(oldName).join(newName);
            fs.writeFileSync(filePath, updatedContent, "utf-8");
        }
    });
});


