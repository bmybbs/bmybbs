import { BMYClient } from "../../utils/BMYClient.js"
import { BMY_EC } from "../../utils/BMYConstants.js"

Page({
	data: {
		login_ok: false,
		user_info: {},
		load_user_meta_interval: null,
	},
	load_user_meta: function() {
		BMYClient.get_user_info("").then(response => {
			if (response.errcode != BMY_EC.API_RT_SUCCESSFUL) {
				this.setData({
					login_ok: false,
				});
			} else {
				this.setData({
					login_ok: true,
					user_info: response,
				});
			}
		});
	},
	onLoad() {
		this.load_user_meta();

		this.setData({
			load_user_meta_interval: setInterval(this.load_user_meta, 60 * 1000),
		});
	},
	onUnload() {
		if (this.data.load_user_meta_interval) {
			clearInterval(this.data.load_user_meta_interval);
			this.setData({
				load_user_meta_interval: null,
			});
		}
	},
})

