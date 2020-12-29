import { BMYClient } from "../../utils/BMYClient.js"

Component({
	properties: {
		board: { type: String },
		aid: { type: Number },
	},
	lifetimes: {
		attached: function() {
			BMYClient.get_article_content(this.data.board, this.data.aid).then(response => {
				response.content = response.content.replace("<article>", "<div>").replace("</article>", "</div>");

				this.setData({
					show_ansi: true,
					article: response,
				});
			});
		},
	},
	methods: {
	},
})

