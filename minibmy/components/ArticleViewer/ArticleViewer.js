import { BMYClient } from "../../utils/BMYClient.js"
import { getLongVersionTime } from "../../utils/Time.js"
import { DOMParser, XMLSerializer } from "xmldom"
import bmyParser from "@bmybbs/bmybbs-content-parser"
import { minibmyFormatArticle, generateContent } from "../../utils/ArticleUtils.js"
import { BMY_EC, RE } from "../../utils/BMYConstants.js"
import { getErrorMessage } from "../../utils/ErrorMsgUtils.js"

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
		doNothing() {
			wx.showToast({
				title: "功能未实现",
				icon: "error",
				duration: 1000,
			});
		},
		openReply() {
			this.triggerEvent("reply", {
				that: this,
				title: `您正在回复${this.data.article.author}网友的帖子《${this.data.article.title}》`,
			}, {});
		},
		doReply(obj /* 与 editor triggerEvent 传递的数据相关*/) {
			if (obj.isReply) {
				const article = {
					board: this.data.board,
					title: this.data.article.title.startsWith(RE) ? this.data.article.title : `${RE}${this.data.article.title}`,
					content: generateContent(obj.body, this.data.article.author, this.data.rawContent),
					anony: false,
					norep: false,
					math: false,
					ref: this.data.aid,
					rid: 0,
				}

				BMYClient.reply_article(article).then(response => {
					if (response.errcode == BMY_EC.API_RT_SUCCESSFUL) {
						if (obj.onSuccess !== null && typeof obj.onSuccess === "function") {
							obj.onSuccess();
						}
					} else {
						wx.showToast({
							title: getErrorMessage(response.errcode),
							icon: "none",
							duration: 2000,
						});
					}
				});
			}
		},
		toggleAnsi() {
			this.setData({
				show_ansi: !this.data.show_ansi
			});

			const article = this.data.article;
			const doc = new DOMParser().parseFromString(article.content, "text/html");
			const ansi_arr = [].slice.call(doc.getElementsByTagName("span"));
			ansi_arr.forEach((x) => {
				let classes = x.getAttribute("class");
				if (this.data.show_ansi) {
					x.setAttribute("class", "bmybbs-ansi " + classes);
				} else {
					x.setAttribute("class", classes.replace("bmybbs-ansi ", ""));
				}
			});

			article.content = new XMLSerializer().serializeToString(doc);
			this.setData({
				article
			});
		}
	},
})

