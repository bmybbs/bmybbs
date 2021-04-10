import { getReadableTime } from "../../utils/Time.js"

Component({
	properties: {
		article: {
			type: Object
		}
	},
	data: {
	},
	attached: function() {
		this.setData({
			readableTime: getReadableTime(this.data.article.tid),
		})
	},
	methods: {
		gotoThread() {
			wx.navigateTo({
				url: `../../pages/thread/thread?boardname_en=${this.data.article.boardname_en}&tid=${this.data.article.tid}`,
			});
		},
		gotoBoard() {
			wx.navigateTo({ url: `../../pages/board/board?boardname=${this.data.article.boardname_en}` });
		},
	}
})

