<!--
	使用方法：
	<SidebarSecList v-bind:_name="" v-bind:_sec_id="" v-bind:_icon=""/>
-->
<template>
	<div class="accordion-item">
		<h2 class="accordion-header">
			<button class="accordion-button collapsed" type="button" data-toggle="collapse" v-bind:data-target="'#sidebar-collapse-' + _sec_id" aria-expanded="false" v-bind:aria-controls="'sidebar-collapse-' + _sec_id">
				<span class="sidebar-icon"><fa v-bind:icon="_icon" /></span> {{ _name }}
			</button>
		</h2>
		<div v-bind:id="'sidebar-collapse-' + _sec_id" class="accordion-collapse collapse">
			<div class="accordion-body">
				<ul class="list-group list-group-flush">
					<li class="list-group-item" v-for="board in boards" v-bind:key="board.name">{{ board.title }}</li>
				</ul>
			</div>
		</div>
	</div>
</template>

<script>
import { BMYClient } from "@/lib/FakeBMYClient.js";

export default {
	data() {
		return {
			boards: [],
		};
	},
	mounted() {
		BMYClient.get_boards_by_section(this._sec_id).then((data) => {
			data.boards.forEach((el) => {
				this.boards.push(el);
			});
		});
	},
	props: {
		_name: String,
		_sec_id: String,
		_icon: String,
	},
}
</script>

<style scoped>
.accordion-button {
	padding: 0.5rem 1.25rem;
	border: 0px;
}

.accordion-button::after {
	width: 0.8rem;
	height: 0.8rem;
	background-size: 0.8rem;
}

.accordion-button:focus {
	border-color: initial;
	box-shadow: none;
}

.accordion-button:not(.collapsed) {
	font-weight: 500;
	color: initial;
	background-color: initial;
	border-left: 5px solid rgba(0,0,0,.4);
	padding-left: -5px;
}

.accordion-collapse {
	border: 0;
}

.accordion-body {
	padding: 0 1.25rem;
	border-left: 5px solid rgba(0,0,0,.4);
}

.list-group {
	padding-left: 26px;
}

.list-group-item {
	background-color: initial;
	padding: 0.2rem 0;
}

.sidebar-icon {
	display: inline-block;
	width: 16px;
	height: 16px;
	color: #6c757daa;
	margin-right: 10px;
}
</style>

