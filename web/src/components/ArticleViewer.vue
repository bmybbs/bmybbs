<template>
	<div class="card border-bmy-blue1 mb-4">
		<div class="card-header bg-bmy-blue0 bg-gradient">
			<div class="d-flex">
				<span class="fw-bold text-bmy-dark8">{{ title }}</span>
				<BadgeArticleFlags :_accessed="_mark" />
			</div>

			<div class="text-bmy-dark6 fs-7">
				{{author}} 发表于 <TooltipTimestamp :_unix_timestamp="_aid" />
			</div>
		</div>
		<div class="card-body">
			<div class="article" v-html="content" @click="toggleAnsi" ref="article"></div>
		</div>
		<div class="card-footer bg-bmy-blue0">
			<div class="action-container d-flex justify-content-between">
				<button v-for="b in buttons" :key="b.text" @click="b.func">
					<span class="me-1"><fa :icon="b.icon" /></span>
					{{ b.text }}
				</button>
			</div>

			<div class="replyForm p-2" v-if="actionForm.isReplying">
				<textarea class="form-control mb-2" ref="replyForm" placeholder="您想说点什么？Remember, be nice..." rows="1"></textarea>
				<div class="d-flex justify-content-between fs-7">
					<button @click="gotoReply">切换为完整编辑框</button>
					<div>
						<button class="me-1" @click="closeForm">取消</button>
						<button @click="doReply">回复</button>
					</div>
				</div>
			</div>
		</div>
	</div>
</template>

<script>
import { defineAsyncComponent } from "vue"
import { BMYClient } from "@/lib/BMYClient.js"
import bmyParser from "@bmybbs/bmybbs-content-parser"
import Prism from "prismjs"
import { BMY_EC, BMY_FILE_HEADER } from "@/lib/BMYConstants.js"
import { generateContent } from "@/lib/BMYUtils.js"
import "@/plugins/mathjax.js"

const TooltipTimestamp = defineAsyncComponent(() => import("./TooltipTimestamp.vue"));
const BadgeArticleFlags = defineAsyncComponent(() => import("./BadgeArticleFlags.vue"));
const RE = "Re: ";

export default {
	data() {
		return {
			ansi_list: [],
			show_ansi: true,
			content: "",
			author: "",
			title: "",
			rawContent: "",
			buttons: [
				{ icon: "comments",          text: "回复", func: this.openReplyForm },
				{ icon: "retweet",           text: "转载", func: this.unimplemented },
				{ icon: "share",             text: "转寄", func: this.unimplemented },
				{ icon: ["far", "envelope"], text: "回信", func: this.unimplemented },
			],
			actionForm: {
				isReplying: false,
			},
		}
	},
	props: {
		_boardname_en: String,
		_aid: Number,
		_mark: Number,
		_reply_callback: Function,
	},
	mounted() {
		BMYClient.get_article_content(this._boardname_en, this._aid).then(response => {
			this.rawContent = response.content;
			this.content = bmyParser({
				text: response.content,
				attaches: response.attach
			});
			this.author = response.author;
			this.title = response.title;
			setTimeout(() => {
				this.ansi_list = [].slice.call(this.$refs.article.querySelectorAll("span.bmybbs-ansi"));
				Prism.highlightAll();
				if (this._mark & BMY_FILE_HEADER.FH_MATH) {
					if (window.MathJax && typeof window.MathJax.typeset === "function") {
						window.MathJax.typeset();
					}
				}
			}, 1500);
		});
	},
	methods: {
		toggleAnsi() {
			if (this.show_ansi) {
				this.show_ansi = false;
				this.ansi_list.forEach((x) => {
					x.classList.remove("bmybbs-ansi");
				});
			} else {
				this.show_ansi = true;
				this.ansi_list.forEach((x) => {
					x.classList.add("bmybbs-ansi");
				});
			}
		},
		unimplemented() {
			this.$toast.error("功能未实现", {
				position: "top"
			});
		},
		closeForm() {
			this.actionForm.isReplying = false;
		},
		openReplyForm() {
			this.closeForm();

			this.actionForm.isReplying = true;
		},
		gotoReply() {
			this.bmy_cache.article = {
				board: this._boardname_en,
				aid: this._aid,
				title: this.title.startsWith(RE) ? this.title : `${RE} ${this.title}`,
				content: generateContent(this.$refs.replyForm.value, this.author, this.rawContent),
			};
			this.$router.push({
				name: "reply",
				params: {
					boardname: this._boardname_en,
					aid: this._aid,
				}
			});
		},
		doReply() {
			const el = this.$refs.replyForm;
			const content = el.value.replaceAll("[ESC][", "\x1b[");
			if (content.length == 0) {
				this.$toast.warning("您并没有写些什么...", {
					position: "top"
				});
				return;
			}

			const article = {
				board: this._boardname_en,
				title: this.title.startsWith(RE) ? this.title : `${RE} ${this.title}`,
				content: generateContent(content, this.author, this.rawContent),
				anony: false,
				norep: false,
				math: false,
				ref: this._aid,
				rid: 0,
			};

			BMYClient.reply_article(article).then(response => {
				if (response.errcode == BMY_EC.API_RT_SUCCESSFUL) {
					el.value = "";
					this.closeForm();
					this.$toast.success("发布成功", {
						position: "top"
					});
					this._reply_callback();
				} else {
					let msg = "未知错误";

					switch (response.errcode) {
					case BMY_EC.API_RT_NOTLOGGEDIN:  msg = "请先登录";       break;
					case BMY_EC.API_RT_WRONGPARAM:   msg = "参数错误";       break;
					case BMY_EC.API_RT_ATCLNOTITLE:  msg = "缺少标题";       break;
					case BMY_EC.API_RT_NOSUCHBRD:    msg = "版面不存在";     break;
					case BMY_EC.API_RT_NOSUCHUSER:   msg = "用户不存在";     break;
					case BMY_EC.API_RT_NOBRDPPERM:   msg = "您被禁止发帖";   break;
					case BMY_EC.API_RT_CNTMAPBRDIR:  msg = "版面内部错误";   break;
					case BMY_EC.API_RT_ATCLFBDREPLY: msg = "本文不可回复";   break;
					case BMY_EC.API_RT_USERLOCKFAIL: msg = "用户内部错误";   break;
					case BMY_EC.API_RT_WRONGTOKEN:   msg = "请再试一次";     break;
					case BMY_EC.API_RT_FBDGSTIP:     msg = "GUEST 无法发帖"; break;
					case BMY_EC.API_RT_ATCLINNERR:   msg = "文章内部错误";   break;
					}

					this.$toast.error(msg, {
						position: "top"
					});
				}
			});
		},
	},
	components: {
		BadgeArticleFlags,
		TooltipTimestamp,
	},
}
</script>

<style>
mjx-container {
	font-size: 120%
}
</style>

<style scoped>
.article:deep(img) {
	max-width: 100%
}

.article:deep(blockquote) {
	padding: 0.5rem 1rem;
	border-left: 5px solid var(--bs-bmy-grey2);
	color: var(--bs-gray);
	background: var(--bs-bmy-grey1);
}

.article:deep(blockquote p) {
	margin-bottom: 0.1rem;
	margin-block-end: 0.1rem;
}

.card-footer button {
	border: 0px;
	padding: 0.5rem 1rem;
	border-radius: 1.25rem;
	background-color: var(--bs-bmy-blue0);
	color: var(--bs-secondary);
}

.card-footer button:hover {
	background-color: var(--bs-bmy-grey1);
}

.action-container button {
	font-weight: 700;
	font-size: 0.8rem;
}

.replyForm textarea {
	height: 4rem;
	width: 100%;
}

.replyForm textarea::placeholder {
	font-size: 0.9rem;
}

.replyForm button {
	font-weight: 700;
	padding: 0.3rem 0.8rem;
	border: 2px solid var(--bs-bmy-grey2);
}

</style>

