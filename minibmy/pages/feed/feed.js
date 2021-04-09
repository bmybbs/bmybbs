import { BMYClient } from "../../utils/BMYClient.js"
import { BMY_EC } from "../../utils/BMYConstants.js"

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
			page: page + 1,
		});
	}
};

Page({
	data: {
		articles: [],
		boards: [],
		feedSet: new Set(),
		page: 1,
		activeTab: 0,
		tabs: [
			{ title: "话题" },
			{ title: "收藏夹列表" },
		],
		login_ok: true,
	},
	load_feed: function() {
		this.get_list(1, false, () => this.get_boards());
	},
	get_list(page, append = false, callback = null) {
		BMYClient.get_feed(page).then(response => {
			switch (response.errcode) {
			case BMY_EC.API_RT_SUCCESSFUL:
				update_article_callback(
					response,
					append ? this.data.articles : [],
					append ? this.data.feedSet  : new Set(),
					page,
					this
				);

				if (callback !== null && typeof callback === "function") {
					callback();
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
	get_boards() {
		BMYClient.get_fav_board_list().then(response => {
			if (response.errcode == BMY_EC.API_RT_SUCCESSFUL && Array.isArray(response.board_array)) {
				this.setData({ boards: response.board_array });
			}
		});
	},
	onLoad: function() {
		this.load_feed();
	},
	gotoLogin() {
		wx.reLaunch({
			url: "../index/index"
		});
	},
	onTabClick(e) {
		this.setData({ activeTab: e.detail.index });
	},
	onChange(e) {
		this.setData({ activeTab: e.detail.index });
	},
	onFeedEnd() {
		this.get_list(this.data.page, true);
	},
	onFeedRefresh() {
		this.get_list(this.data.page, false, () => this.get_boards());
	}
})

