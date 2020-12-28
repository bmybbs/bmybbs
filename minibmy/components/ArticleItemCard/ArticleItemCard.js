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
			readableTime: dayjs(this.data.article.aid * 1000).fromNow(),
		})
	},
	methods: {
	}
})

