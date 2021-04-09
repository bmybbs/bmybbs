import { getReadableTime } from "../../utils/Time.js"

Component({
	properties: {
		postItem: {
			type: Object
		}
	},
	data: {
	},
	attached() {
		this.setData({
			readableTime: getReadableTime(this.data.postItem.tid),
		});
	},
	methods: {
		gotoThread() {
			wx.navigateTo({
				url: `../../pages/thread/thread?boardname_en=${this.data.postItem.board}&tid=${this.data.postItem.tid}`,
			});
		},
	}
})

