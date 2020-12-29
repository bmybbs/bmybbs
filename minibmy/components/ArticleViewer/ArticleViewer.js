import { BMYClient } from "../../utils/BMYClient.js"
import { getLongVersionTime } from "../../utils/Time.js"
import { DOMParser, XMLSerializer } from "xmldom"

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
					time: getLongVersionTime(this.data.aid),
					article: response,
				});
			});
		},
	},
	methods: {
		toggleAha() {
			this.setData({
				show_ansi: !this.data.show_ansi
			});

			const doc = new DOMParser().parseFromString(this.data.article.content, "text/html");
			const aha_arr = [].slice.call(doc.getElementsByTagName("span"));
			aha_arr.map((x) => {
				let classes = x.getAttribute("class");
				if (this.data.show_ansi) {
					x.setAttribute("class", "aha " + classes);
				} else {
					x.setAttribute("class", classes.replace("aha ", ""));
				}
			});

			let article = this.data.article;
			article.content = new XMLSerializer().serializeToString(doc);
			this.setData({
				article
			});
		}
	},
})

