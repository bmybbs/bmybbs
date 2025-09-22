import { defineConfig } from "vite";
import vue from "@vitejs/plugin-vue";
import prismjsOption from "./prismjs.config.js";
import { BundlePrismjs } from "rollup-plugin-prismjs"
import path from "path";

import fs from "fs";
const proxy_config_file = path.resolve(__dirname, "./proxy.config.txt");

let PROXY_CONFIG;
if (!fs.existsSync(proxy_config_file)) {
	console.warn("ERROR: Proxy config file doesn't exist, will use the main site as backend");
	console.warn("       To create this config file, here's an example");
	console.warn("       echo 'http://ip:port' > /path/to/web/proxy.config.txt");
	console.warn("       where bmyapi listens on ip:port");
	PROXY_CONFIG = "https://bbs.xjtu.edu.cn";
} else {
	PROXY_CONFIG = fs.readFileSync(proxy_config_file, "utf8").trim();
}

export default defineConfig({
	plugins: [
		vue(),
		BundlePrismjs(prismjsOption),
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
