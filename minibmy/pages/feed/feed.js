import { BMYClient } from "../../utils/BMYClient.js"
import { BMY_EC, ITEMS_PER_PAGE } from "../../utils/BMYConstants.js"

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
			total: Math.ceil(response.total / ITEMS_PER_PAGE),
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
		total: 1,
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
		if (this.data.page <= this.data.total) {
			this.get_list(this.data.page, true);
		} else {
			wx.showToast({
				title: "已到达最后一篇啦",
				icon: "none",
				duration: 2000,
			});
		}
	},
	onFeedRefresh() {
		this.get_list(this.data.page, false, () => this.get_boards());
	}
})

