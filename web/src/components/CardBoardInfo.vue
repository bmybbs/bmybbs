<template>
	<div class="card border-bmy-blue1" @mouseenter="onMouseOver" @mouseout="onMouseOut" :class="{ 'prevent-events': _prevent_events, 'in-popover': _in_popover }">
		<div class="card-header bg-bmy-blue1 bg-gradient text-white title d-flex justify-content-between">
			<span>
				<span @click="toggleFav" style="cursor: pointer">
					<fa :icon="['fas', 'star']" v-if="info.is_fav == 1" />
					<fa :icon="['far', 'star']" v-else />
				</span>
				{{ info.zh_name }}
			</span>
			<span class="font-monospace">{{ info.secstr }}/{{ info.name }}</span>
		</div>

		<div class="card-body" style="z-index: 0">
			<div class="d-flex justify-content-between flex-wrap mb-3">
				<div class="numbers-item-wrapper d-flex align-items-center mb-1" v-for="i in infoArray" :key="i.name">
					<div class="ratio ratio-1x1 m-2">
						<div class="d-flex flex-column align-items-center justify-content-center">
							<div class="numbers-item-number">{{ i.num }}</div>
							<div class="numbers-item-text">{{ i.name }}</div>
						</div>
					</div>
				</div>
			</div>

			<div class="intro block" v-if="info.notes && info.notes.length > 0">
				<div class="bmy-card-heading">简介</div>
				<div class="my-3" v-html="mdNotes"></div>
			</div>

			<div class="keyword block">
				<div class="bmy-card-heading">关键字</div>
				<div class="my-3"></div>
			</div>

			<div class="hot block">
				<div class="bmy-card-heading">热门话题</div>
				<div class="my-3"></div>
			</div>
		</div>
	</div>
</template>

<script>
import DOMPurify from "dompurify";
import marked from "marked";
import { BMYClient } from "@/lib/BMYClient.js"
import { BMY_EC } from "@/lib/BMYConstants.js"

const kFormatter = num => {
	if (num > 999)
		return (num/1000).toFixed(1) + "k";
	else
		return num;
};

export default {
	data() {
		return {
			info: {
			},
		};
	},
	created() {
		this.$watch(() => this._boardname_en, (toBoardName) => {
			this.get_info(toBoardName);
		});
	},
	mounted() {
		this.get_info(this._boardname_en);
	},
	computed: {
		infoArray() {
			return [
				{ name: "主题", num: this.info.thread_num ? kFormatter(this.info.thread_num) : 0 },
				{ name: "帖子", num: this.info.article_num ? kFormatter(this.info.article_num) : 0 },
				{ name: "新增", num: this.info.today_new ? kFormatter(this.info.today_new) : 0 },
				{ name: "在线", num: this.info.inboard_num ? kFormatter(this.info.inboard_num) : 0 },
			];
		},
		mdNotes() {
			if (this.info.notes && this.info.notes.length > 0) {
				return DOMPurify.sanitize(marked(this.info.notes));
			} else {
				return "";
			}
		},
	},
	props: {
		_boardname_en: String,
		_prevent_events: {
			type: Boolean,
			default: false
		},
		_in_popover: {
			type: Boolean,
			default: false
		},
		_events: Object,
	},
	methods: {
		get_info(boardname) {
			BMYClient.get_board_info(boardname).then(response => {
				this.info = response;
			});
		},
		onMouseOver() {
			if (this._events && typeof(this._events.mouseover) === "function") {
				this._events.mouseover();
			}
		},
		onMouseOut() {
			if (this._events && typeof(this._events.mouseout) === "function") {
				this._events.mouseout();
			}
		},
		toggleFav() {
			if (this.info.is_fav == 1) {
				BMYClient.fav_del(this._boardname_en).then(response => {
					if (response.errcode == 0 || response.errcode == BMY_EC.API_RT_NOTINRCD) {
						this.$toast.success("移除成功", {
							position: "top"
						});
						this.info.is_fav = 0;
					} else if (response.errcode == BMY_EC.API_RT_NOTLOGGEDIN) {
						this.$toast.error("请重新登录", {
							position: "top"
						});
					}
				});
			} else {
				BMYClient.fav_add(this._boardname_en).then(response => {
					const cfg = {
						type: "error",
						position: "top",
					};
					let msg = "";

					switch (response.errcode) {
						case BMY_EC.API_RT_SUCCESSFUL:
						case BMY_EC.API_RT_ALRDYINRCD:
							msg = "添加成功";
							cfg.type = "success";
							this.info.is_fav = 1;
							break;
						case BMY_EC.API_RT_NOTLOGGEDIN:
							msg = "请重新登录";
							break;
						case BMY_EC.API_RT_REACHMAXRCD:
							msg = "已达到收藏夹上限，无法添加";
							break;
						case BMY_EC.API_RT_NOSUCHBRD:
						case BMY_EC.API_RT_FBDNUSER:
							msg = "找不到这个版面";
							break;
						default:
							break;
					}

					this.$toast.show(msg, cfg);
				});
			}
		},
	},
}
</script>

<style scoped>
.prevent-events * {
	pointer-events: none;
}

.in-popover {
	width: 320px;
}

.numbers-item-number {
	font-size: 18px;
	font-weight: 600;
}

.numbers-item-text {
	font-size: 12px;
}

.bmy-card-heading {
	margin: 0 -1rem;
	padding: 0.5rem 1rem;
	background: var(--bs-bmy-blue0);
	border-top: 1px solid var(--bs-bmy-blue);
	border-bottom: 1px solid var(--bs-bmy-blue);
}

@media (max-width: 576px) {
	.numbers-item-wrapper {
		width: 22.5%;
	}
}

@media (min-width: 576px) {
	.numbers-item-wrapper {
		width: 48%;
	}
}

@media (min-width: 1200px) {
	.numbers-item-wrapper {
		width: 22.5%;
	}
}

.in-popover .numbers-item-wrapper {
	width: 22.5% !important;
}
</style>

<style lang="scss" scoped>
/* credit: https://css-tricks.com/gradient-borders-in-css/ */
.numbers-item-wrapper {
	$border: 5px;
	border: $border solid transparent;
	border-radius: 50%;
	background: white;
	background-clip: padding-box;
	position: relative;
	&:before {
		content: '';
		position: absolute;
		top: 0; right: 0; bottom: 0; left: 0;
		z-index: -1;
		margin: -$border;
		border-radius: inherit;
		background: linear-gradient(45deg, #267dcf 0%, #74fac8 95%);
	}
}
</style>

