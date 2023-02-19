import { defineConfig } from "vite";
import vue from "@vitejs/plugin-vue";

import path from "path";

import fs from "fs";
const proxy_config_file = path.resolve(__dirname, "./proxy.config.txt");

if (!fs.existsSync(proxy_config_file)) {
	console.error("ERROR: Proxy config file doesn't exist!");
	console.error("       To create this config file, here's an example");
	console.error("       echo 'http://ip:port' > /path/to/web/.proxy.txt");
	console.error("       where bmyapi listens on ip:port");
	process.exit();
}

const PROXY_CONFIG = fs.readFileSync(proxy_config_file, "utf8").trim();

export default defineConfig({
	plugins: [
		vue(),
	],
	resolve: {
		alias: {
			"@": path.resolve(__dirname, "./src"),
		},
	},
	server: {
		host: true,
		port: 4000,
		proxy: {
			"^/BMY" : {
				target: PROXY_CONFIG,
			},
			"^/bmyMainPic": {
				target: PROXY_CONFIG,
			},
			"^/api": {
				target: PROXY_CONFIG,
			}
		},
	},
});

