import { BMYClient } from "../../utils/BMYClient.js"
import { BOARD_ARTICLE_MODE, ITEMS_PER_PAGE } from "../../utils/BMYConstants.js"

const update_article_callback = (response, article_arr, set, page, that) => {
	if (Array.isArray(response.articlelist)) {
		for (let i = response.articlelist.length - 1, x; i >= 0; i--) {
			x = response.articlelist[i];
			if (!set.has(x.aid)) {
				set.add(x.aid);
				article_arr.push(x);
			}
		}

		that.setData({
			articles: article_arr,
			feedSet: set,
			page: page + 1,
			total: Math.ceil(response.total / ITEMS_PER_PAGE)
		});
	}
};

Page({
	data: {
		articles: [],
		board: {},
		mdNotes: "",
		moderators: [],
		viceModerators: [],
		keywords: [],
		feedSet: new Set(),
		page: 1,
		activeTab: 0,
		tabs: [
			{ title: "话题", },
			{ title: "版面介绍", }
		]
	},
	onLoad: function (options) {
		BMYClient.get_board_info(options.boardname).then(response => {
			this.updateNotes(response.notes);
			this.updateModerators(response.bm);
			this.updateKeywords(response.keyword);
			this.setData({
				board: response
			});
		});

		this.get_list(options.boardname, BOARD_ARTICLE_MODE.THREAD_MODE, 1);
	},
	updateNotes(notes) {
		if (notes && notes.length > 0) {
			this.setData({
				mdNotes: notes
			});
		}
	},
	updateModerators(arr) {
		const bm = [], vbm = [];
		let i;
		if (Array.isArray(arr)) {
			for (i = 0; i < 4; i++) {
				if (arr[i])
					bm.push(arr[i]);
			}

			for (i = 4; i < arr.length; i++) {
				if (arr[i])
					vbm.push(arr[i]);
			}
		}
		this.setData({
			moderators: bm,
			viceModerators: vbm,
		});
	},
	updateKeywords(keyword) {
		if (keyword && keyword.length > 0) {
			this.setData({
				keywords: keyword.split(/[\s,;.:]+/)
			});
		}
	},
	get_list(boardname, mode, page, append = false, callback = null) {
		BMYClient.get_article_list_by_board(boardname, mode, page).then(response => {
			if (response.errcode == 0) {
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
	onArticleEnd() {
		if (this.data.page <= this.data.total)
			this.get_list(this.data.board.name, BOARD_ARTICLE_MODE.THREAD_MODE, this.data.page, true);
		else
			wx.showToast({
				title: "已到达最后一篇啦",
				icon: "none",
				duration: 2000,
			});
	},
	onArticleRefresh() {
		this.get_list(this.data.board.name, BOARD_ARTICLE_MODE.THREAD_MODE, 1, false, () => {
			this.setData({
				articleRefreshTriggered: false
			});
		});
	},
})

