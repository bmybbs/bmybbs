<template>
	<ul>
		<li v-for="article in _articles" :key="article.aid">
			{{ article.title }} By {{ article.author }} at {{ article.board }}
		</li>
	</ul>
</template>

<script>
export default {
	data() {
	},
	props: {
		_articles: Array,
	},
}
</script>

