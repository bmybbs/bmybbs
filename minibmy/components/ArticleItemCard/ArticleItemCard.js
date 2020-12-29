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
	}
})

