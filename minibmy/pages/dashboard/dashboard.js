import { BMYClient } from "../../utils/BMYClient.js"

const create_article_id = (el) => {
	el.id = el.board + el.aid;
};

Page({
	data: {
		announce_list: [],
		commend_list: [],
		top10: [],
	},
	onLoad: function() {
		BMYClient.get_announce().then(response => {
			if (response.errcode == 0 && Array.isArray(response.articlelist)) {
				response.articlelist.forEach(create_article_id);
				this.setData({
					announce_list: response.articlelist
				});
			}
		});

		BMYClient.get_commend().then(response => {
			if (response.errcode == 0 && Array.isArray(response.articlelist)) {
				response.articlelist.forEach(create_article_id);
				this.setData({
					commend_list: response.articlelist
				});
			}
		});

		BMYClient.get_top10().then(response => {
			if (response.errcode == 0 && Array.isArray(response.articlelist)) {
				response.articlelist.forEach(create_article_id);
				this.setData({
					top10: response.articlelist
				});
			}
		});
	},
});

