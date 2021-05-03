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
		<ul class="dropdown-menu dropdown-menu-dark" :class="{ show: realShow }">
			<li v-if="boards.length > 0">
				<h6 class="dropdown-header">找到的版面有：</h6>
				<ul class="search-results">
					<li class="dropdown-item" v-for="board in boards" v-bind:key="board.name" @click="gotoBoard(board.name)">
						<NavSearchHighlightItem v-bind:_text="board.name" v-bind:_key="search_str" />
					</li>
					<li class="dropdown-item" v-if="hasMoreBoards">更多版面...</li>
				</ul>
			</li>
			<li v-if="boards.length > 0 && users.length > 0"><hr class="dropdown-divider"></li>
			<li v-if="users.length > 0">
				<h6 class="dropdown-header">找到的用户有：</h6>
				<ul class="search-results">
					<li class="dropdown-item" v-for="user in users" v-bind:key="user">
						<NavSearchHighlightItem v-bind:_text="user" v-bind:_key="search_str" />
					</li>
					<li class="dropdown-item" v-if="hasMoreUsers">更多用户...</li>
				</ul>
			</li>
			<li v-if="searchContent">
				<h6 class="dropdown-header" @click="gotoSearchContent">搜索包含"{{ search_str }}"的文章</h6>
			</li>
		</ul>
	</div>
</template>

<script>
import { BMYClient } from "@/lib/BMYClient.js"
import NavSearchHighlightItem from "@/components/NavSearchHighlightItem.vue"

const MAXRECORDS = 5;
const VALID_BMYID_REG = /^[a-zA-Z0-9_]*$/;

export default {
	data() {
		return {
			dropdown: null,
			show: false,
			search_str: "",
			boards: [],
			users: [],
			lazySearchTimer: null,
			hasMoreUsers: false,
			hasMoreBoards: false,
			searchContent: false,
		};
	},
	mounted() {
	},
	components: {
		NavSearchHighlightItem,
	},
	methods: {
		showDropdown() {
			this.show = true;
		},
		closeDropdown() {
			setTimeout(() => {
				this.show = false;
			}, 200);
		},
		doLasySearch() {
			if (this.lazySearchTimer) {
				clearTimeout(this.lazySearchTimer);
			}

			this.lazySearchTimer = setTimeout(() => {
				if (this.search_str.length == 0) {
					this.boards.length = 0;
					this.users.length = 0;
					this.searchContent = false;
				} else {
					if (VALID_BMYID_REG.test(this.search_str)) {
						BMYClient.search_board(this.search_str).then(response => {
							if (response.errcode == 0 && Array.isArray(response.board_array)) {
								this.boards = response.board_array;
								this.hasMoreBoards = this.boards.length > MAXRECORDS;
								if (this.hasMoreBoards)
									this.boards.length = MAXRECORDS;
							} else {
								this.boards.length = 0;
								this.hasMoreBoards = false;
							}
						});

						BMYClient.search_user(this.search_str).then(response => {
							if (response.errcode == 0 && Array.isArray(response.user_array)) {
								this.users = response.user_array;
								this.hasMoreUsers = this.users.length > MAXRECORDS;
								if (this.hasMoreUsers)
									this.users.length = MAXRECORDS;
							} else {
								this.users.length = 0;
								this.hasMoreUsers = false;
							}
						});
					}

					if (["board", "boardSubmit", "thread", "reply", "boardSearch"].includes(this.$route.name)) {
						this.searchContent = true;
					}
				}
			}, 200);
		},
		gotoBoard(boardname) {
			this.$router.push({
				name: "board",
				params: {
					boardname: boardname,
				}
			});
		},
		gotoSearchContent() {
			this.$router.push({
				name: "boardSearch",
				params: {
					boardname: this.$route.params.boardname,
				},
				query: {
					q: this.search_str,
				},
			});
		},
	},
	computed: {
		realShow() {
			return this.show && this.search_str.length > 0;
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

