import { BMYClient } from "../../utils/BMYClient.js"
import { BOARD_SORT_MODE, BMYSECSTRS } from "../../utils/BMYConstants.js"

Page({
	data: {
		articles: [],
		boards: [],
		sec_en: "",
		sec_zh: "",
		activeTab: 0,
		tabs: [
			{ title: "话题", },
			{ title: "版面", }
		]
	},
	onLoad: function (options) {
		BMYClient.get_article_list_by_section(options.id).then(response => {
			if (Array.isArray(response.articles)) {
				response.articles.forEach((x) => {
					x.id = x.boardname_en + x.tid;
				});
				this.setData({
					articles: response.articles
				});
			}
		});
		BMYClient.get_boards_by_section(options.id, BOARD_SORT_MODE.BY_ALPHABET).then(response => {
			if (response.errcode == 0) {
				this.setData({
					boards: response.boardlist
				});
			}
		});
		BMYSECSTRS.forEach((x) => {
			if (x.id == options.id) {
				this.setData({
					sec_en: x.id,
					sec_zh: x.text,
				});
			}
		})
	},
	onTabClick(e) {
		this.setData({
			activeTab: e.detail.index
		});
	},
	onChange(e) {
		this.setData({
			activeTab: e.detail.index
		});
	},
})

