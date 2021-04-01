import { BMYClient } from "../../utils/BMYClient.js"
import { getLongVersionTime } from "../../utils/Time.js"
import { DOMParser, XMLSerializer } from "xmldom"
import bmyParser from "@bmybbs/bmybbs-content-parser"
import { minibmyFormatArticle } from "../../utils/ArticleUtils.js"

Component({
	properties: {
		board: { type: String },
		aid: { type: Number },
	},
	lifetimes: {
		attached: function() {
			BMYClient.get_article_content(this.data.board, this.data.aid).then(response => {
				const content = bmyParser({
					text: response.content,
					attaches: response.attach
				}).replace("<article>", "<div>").replace("</article>", "</div>");

				this.setData({
					rawContent: response.content,
					show_ansi: true,
					time: getLongVersionTime(this.data.aid),
					article: {
						title: response.title,
						author: response.author,
						content: minibmyFormatArticle(content),
					}
				});
			});
		},
	},
	methods: {
		toggleAha() {
			this.setData({
				show_ansi: !this.data.show_ansi
			});

			const article = this.data.article;
			const doc = new DOMParser().parseFromString(article.content, "text/html");
			const aha_arr = [].slice.call(doc.getElementsByTagName("span"));
			aha_arr.forEach((x) => {
				let classes = x.getAttribute("class");
				if (this.data.show_ansi) {
					x.setAttribute("class", "aha " + classes);
				} else {
					x.setAttribute("class", classes.replace("aha ", ""));
				}
			});

			article.content = new XMLSerializer().serializeToString(doc);
			this.setData({
				article
			});
		}
	},
})

