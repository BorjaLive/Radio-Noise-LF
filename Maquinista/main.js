const { app, BrowserWindow } = require("electron");
const path = require("path");

process.env['ELECTRON_DISABLE_SECURITY_WARNINGS'] = 'true';

app.whenReady()
    .then(() => {
        createWindow();
        app.on("activate", () => {
            if (BrowserWindow.getAllWindows().length === 0) createWindow;
        });
    });

app.on("window-all-closed", () => {
    if (process.platform !== "darwin") app.quit();
});
app.allowRendererProcessReuse = false

function createWindow() {
    const win = new BrowserWindow({
        width: 720,
        height: 480,
        webPreferences: {
            preload: path.join(__dirname, "src/preload.js")
        }
    });
    win.setMenuBarVisibility(false);
    win.loadFile("src/index.html");
}