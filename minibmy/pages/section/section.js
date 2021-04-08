import { BMYClient } from "../../utils/BMYClient.js"
import { BOARD_SORT_MODE, BMYSECSTRS } from "../../utils/BMYConstants.js"

const update_article_callback = (response, article_arr, set, page, that) => {
	if (Array.isArray(response.articles)) {
		response.articles.forEach((x) => {
			x.id = x.boardname_en + x.tid;
			if (!set.has(x.id)) {
				set.add(x.id);
				article_arr.push(x);
			}
		});

		that.setData({
			articles: article_arr,
			feedSet: set,
			page: page + 1
		});
	}
};

Page({
	data: {
		articles: [],
		boards: [],
		sec_en: "",
		sec_zh: "",
		feedSet: new Set(),
		page: 1,
		activeTab: 0,
		tabs: [
			{ title: "话题", },
			{ title: "版面", }
		]
	},
	onLoad: function (options) {
		BMYClient.get_article_list_by_section(options.id, 1).then(response => {
			update_article_callback(response, [], new Set(), 1, this);
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
	onFeedEnd() {
		BMYClient.get_article_list_by_section(this.data.sec_en, this.data.page).then(response => {
			update_article_callback(response, this.data.articles, this.data.feedSet, this.data.page, this);
		});
	},
	onFeedRefresh() {
		BMYClient.get_article_list_by_section(this.data.sec_en, 1).then(response => {
			this.setData({
				feedRefreshTriggered: false
			});

			update_article_callback(response, [], new Set(), 1, this);
		});
	},
})

