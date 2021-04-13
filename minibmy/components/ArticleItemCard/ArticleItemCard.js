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
			readableTime: getReadableTime(this.data.article.aid),
		})
	},
	methods: {
		gotoThread() {
			wx.navigateTo({ url: `../../pages/thread/thread?boardname_en=${this.data.article.board}&tid=${this.data.article.aid}` });
		},
		gotoBoard() {
			wx.navigateTo({ url: `../../pages/board/board?boardname=${this.data.article.board}` });
		}
	}
})

