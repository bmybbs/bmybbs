import { BMYClient } from "../../utils/BMYClient.js"

let articleInstance;

Page({
	data: {
		editorShow: false,
		editorTitle: "",
		board: "",
		tid: "",
		articlelist: [],
		set: new Set(),
	},
	onLoad: function(options) {
		this.setData({ board: options.boardname_en, tid: options.tid });
		this.loadThread(options.boardname_en, options.tid);
	},
	onOpenEditor(e) {
		articleInstance = e.detail && e.detail.that;
		this.setData({
			editorShow: true,
			editorTitle: e.detail.title,
		});
	},
	onPost(e) {
		if (articleInstance) {
			const callback = e.detail.onSuccess;

			e.detail.onSuccess = () => {
				if (callback !== null && typeof callback === "function") {
					callback();
				}
				// 追加自身的回调
				this.loadThread(this.data.board, this.data.tid, true);
			}

			articleInstance.doReply(e.detail);
		}
	},
	loadThread(board, tid, append = false) {
		const arr = append ? this.data.articlelist : [],
			set   = append ? this.data.set         : new Set();

		BMYClient.get_thread_list(board, tid).then(response => {
			if (response.errcode == 0 && Array.isArray(response.articlelist) && response.articlelist.length > 0) {
				response.articlelist.forEach(x => {
					if (!set.has(x.aid)) {
						arr.push(x);
						set.add(x.aid);
					}
				});

				this.setData({
					articlelist: arr,
					set: set,
				});
			}
		});
	}
});

