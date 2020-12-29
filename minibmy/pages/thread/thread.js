import { BMYClient } from "../../utils/BMYClient.js"

Page({
	data: {
	},
	onLoad: function(options) {
		BMYClient.get_thread_list(options.boardname_en, options.tid).then(response => {
			if (response.errcode == 0 && Array.isArray(response.articlelist) && response.articlelist.length > 0) {
				this.setData({
					thread_title: response.articlelist[0].title,
					boardname_en: response.articlelist[0].board,
					articlelist: response.articlelist
				});
			}
		});
	},
});

