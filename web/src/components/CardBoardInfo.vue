<template>
	<div class="card border-bmy-blue1" @mouseenter="onMouseOver" @mouseout="onMouseOut" :class="{ 'prevent-events': _prevent_events, 'in-popover': _in_popover }" style="z-index: -2">
		<div class="card-header bg-bmy-blue1 bg-gradient text-white title d-flex justify-content-between">
			<span>
				<span>
					<fa :icon="['fas', 'star']" v-if="info.is_fav == 1" />
					<fa :icon="['far', 'star']" v-else />
				</span>
				{{ info.zh_name }}
			</span>
			<span class="font-monospace">{{ info.secstr }}/{{ info.name }}</span>
		</div>

		<div class="card-body">
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

			<div class="intro block">
				<h3>简介</h3>
				<p></p>
			</div>

			<div class="keyword block">
				<h3>关键字</h3>
			</div>

			<div class="hot block">
				<h3>热门话题</h3>
			</div>
		</div>
	</div>
</template>

<script>
import { BMYClient } from "@/lib/BMYClient.js"

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
		}
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
		}
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

h3 {
	font-size: 14px;
	font-weight: 600;
	color: #1c1c1c;
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
		background: linear-gradient(45deg, #267dcf 0%, #74fac8 100%);
	}
}
</style>

