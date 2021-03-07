<template>
	<div>
		<div class="title">
			<span class="title-main">{{ _boardname_zh }}</span>
			<span class="title-sec">{{ _secstr }}/{{ _boardname_en }}</span>
		</div>

		<div class="d-flex justify-content-between mt-2">
			<div class="d-flex flex-column align-items-center" v-for="info in infoArray" :key="info.name">
				<div class="numbers-item-number">{{ info.num }}</div>
				<div class="numbers-item-text">{{ info.name }}</div>
			</div>
		</div>

		<div class="intro block">
			<h3>简介</h3>
			<p>{{ _intro }}</p>
		</div>

		<div class="keyword block">
			<h3>关键字</h3>
		</div>

		<div class="hot block">
			<h3>热门话题</h3>
		</div>
	</div>
</template>

<script>
const kFormatter = num => {
	if (num > 999)
		return (num/1000).toFixed(1) + "k";
	else
		return num;
};

export default {
	data() {
		return {
			infoArray: [
				{ name: "主题", num: kFormatter(this._thread_num) },
				{ name: "帖子", num: kFormatter(this._article_num) },
				{ name: "新增", num: kFormatter(this._today_new) },
				{ name: "在线", num: kFormatter(this._inboard_num) },
			],
		};
	},
	props: {
		_boardname_en: String,
		_boardname_zh: String,
		_secstr: String,
		_article_num: Number,
		_thread_num: Number,
		_today_new: Number,
		_inboard_num: Number,
	},
	methods: {
	},
}
</script>

<style scoped>
.title-main {
	font-size: 20px;
	font-weight: 600;
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

</style>

