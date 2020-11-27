<template>
	<section class="card bmy-dashboard-card">
		<div class="card-header">
			{{ _name }}
		</div>
		<ul class="list-group list-group-flush">
			<li class="list-group-item" v-for="article in _articles" :key="article.aid">
				{{ article.title }} By {{ article.author }} at {{ article.board }}
			</li>
		</ul>
	</section>
</template>

<script>
export default {
	data() {
		return { };
	},
	props: {
		_name: String,
		_articles: Array,
	},
}
</script>

