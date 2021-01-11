import { BMYClient } from "../../utils/BMYClient.js"
import { BMY_EC } from "../../utils/BMYConstants.js"

Page({
	data: {
		articles: [],
		login_ok: true,
	},
	load_feed: function() {
		let time = Math.floor(new Date().getTime() / 1000);
		BMYClient.get_feed(time).then(response => {
			switch (response.errcode) {
			case BMY_EC.API_RT_SUCCESSFUL:
				if (Array.isArray(response.articles)) {
					response.articles.map((x) => {
						x.id = x.boardname_en + x.tid;
					});
					this.setData({
						articles: response.articles
					});
				}
				break;
			case BMY_EC.API_RT_NOTLOGGEDIN:
				this.setData({
					login_ok: false
				});
				break;
			}
		});
	},
	onLoad: function() {
		this.load_feed();
	},
})

