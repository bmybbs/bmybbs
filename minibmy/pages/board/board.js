import insane from "insane";
import marked from "marked";
import { BMYClient } from "../../utils/BMYClient.js"
import { BOARD_ARTICLE_MODE, ITEMS_PER_PAGE, BMY_EC } from "../../utils/BMYConstants.js"
import { getErrorMessage } from "../../utils/ErrorMsgUtils.js"

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
		feedSet: new Set(),
		page: 1,
		editorShow: false,
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
			this.setData({
				board: response
			});
		});

		this.get_list(options.boardname, BOARD_ARTICLE_MODE.THREAD_MODE, 1);
	},
	onShareAppMessage() {
		return {
			title: `兵马俑BBS${this.data.board.zh_name}版(${this.data.board.name})`,
			path: `/pages/board/board?boardname=${this.data.board.name}`,
		}
	},
	onShareTimeline() {
		return {
			title: `兵马俑BBS${this.data.board.zh_name}版(${this.data.board.name})`,
		}
	},
	onPost(e) {
		const article = {
			board: this.data.board.name,
			title: e.detail.title,
			content: e.detail.body.replaceAll("[ESC][", "\x1b["),
			anony: false,
			norep: false,
			math: false,
		};

		BMYClient.post_article(article).then(response => {
			if (response.errcode == BMY_EC.API_RT_SUCCESSFUL) {
				if (e.detail.onSuccess !== null && typeof e.detail.onSuccess === "function") {
					e.detail.onSuccess();
				}
				wx.navigateTo({
					url: `../thread/thread?boardname_en=${this.data.board.name}&tid=${response.aid}`,
				});
			} else {
				wx.showToast({
					title: getErrorMessage(response.errcode),
					icon: "none",
					duration: 2000,
				});
			}
		});
	},
	openEditor() {
		this.setData({ editorShow: true });
	},
	updateNotes(notes) {
		if (notes && notes.length > 0) {
			this.setData({
				mdNotes: insane(marked(notes))
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

