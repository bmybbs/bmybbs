import dayjs from "dayjs";
import relativeTime from "dayjs/plugin/relativeTime"
import localizedFormat from "dayjs/plugin/localizedFormat"
import "dayjs/locale/zh-cn"

dayjs.locale("zh-cn");
dayjs.extend(relativeTime);
dayjs.extend(localizedFormat);

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
			readableTime: dayjs(this.data.article.tid * 1000).fromNow(),
		})
	},
	methods: {
		gotoThread() {
			wx.navigateTo({
				url: `../../pages/thread/thread?boardname_en=${this.data.article.boardname_en}&tid=${this.data.article.tid}`,
			});
		},
	}
})

