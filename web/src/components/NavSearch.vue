<template>
	<div class="dropdown">
		<input ref="input"
			class="form-control form-control-dark dropdown-toggle"
			type="text"
			placeholder="搜索用户、版面"
			v-model="search_str"
			@keyup="doLasySearch"
			@focus="showDropdown"
			@blur="closeDropdown"
		>
		<ul class="dropdown-menu dropdown-menu-dark">
			<li v-if="boards.length > 0">
				<h6 class="dropdown-header">找到的版面有：</h6>
				<ul class="search-results">
					<li class="dropdown-item" v-for="board in boards" v-bind:key="board.name">{{ board.name }}</li>
					<li class="dropdown-item" v-if="hasMoreBoards">更多版面...</li>
				</ul>
			</li>
			<li v-if="boards.length > 0 && users.length > 0"><hr class="dropdown-divider"></li>
			<li v-if="users.length > 0">
				<h6 class="dropdown-header">找到的用户有：</h6>
				<ul class="search-results">
					<li class="dropdown-item" v-for="user in users" v-bind:key="user">{{ user }}</li>
					<li class="dropdown-item" v-if="hasMoreUsers">更多用户...</li>
				</ul>
			</li>
		</ul>
	</div>
</template>

<script>
import Dropdown from "bootstrap/js/dist/dropdown"
import { BMYClient } from "@/lib/BMYClient.js"

const MAXRECORDS = 5;

export default {
	data() {
		return {
			dropdown: null,
			search_str: "",
			boards: [],
			users: [],
			lazySearchTimer: null,
			hasMoreUsers: false,
			hasMoreBoards: false,
		};
	},
	mounted() {
		if (this.dropdown == null) {
			this.dropdown = new Dropdown(this.$refs.input);
			this.dropdown.hide();
		}
	},
	methods: {
		showDropdown() {
			if (this.dropdown)
				this.dropdown.show();
		},
		closeDropdown() {
			if (this.dropdown)
				this.dropdown.hide();
		},
		doLasySearch() {
			if (this.lazySearchTimer) {
				clearTimeout(this.lazySearchTimer);
			}

			const that = this;
			this.lazySearchTimer = setTimeout(function() {
				if (that.search_str.length == 0) {
					that.boards.length = 0;
					that.users.length = 0;
				} else {
					BMYClient.search_board(that.search_str).then(response => {
						if (response.errcode == 0 && Array.isArray(response.board_array)) {
							that.boards = response.board_array;
							that.hasMoreBoards = that.boards.length > MAXRECORDS;
							if (that.hasMoreBoards)
								that.boards.length = MAXRECORDS;
						} else {
							that.boards.length = 0;
							that.hasMoreBoards = false;
						}
					});

					BMYClient.search_user(that.search_str).then(response => {
						if (response.errcode == 0 && Array.isArray(response.user_array)) {
							that.users = response.user_array;
							that.hasMoreUsers = that.users.length > MAXRECORDS;
							if (that.hasMoreUsers)
								that.users.length = MAXRECORDS;
						} else {
							that.users.length = 0;
							that.hasMoreUsers = false;
						}
					});
				}
			}, 200);
		},
	},
}
</script>

<style scoped>
.form-control {
	padding: .75rem 1rem;
	border-width: 0;
	border-radius: 0;
}

.form-control-dark {
	color: #fff;
	background-color: rgba(255, 255, 255, .1);
	border-color: rgba(255, 255, 255, .1);
}

.form-control-dark:focus {
	border-color: transparent;
	box-shadow: 0 0 0 3px rgba(255, 255, 255, .25);
}

.dropdown-menu {
	min-width: 300px;
}

.search-results {
	padding-left: 0px;
}
</style>

