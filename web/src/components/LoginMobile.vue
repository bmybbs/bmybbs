<template>
	<div class="bg-image"></div>
	<div class="bmy-container">
		<div v-if="_checked">
			<div class="form-container" v-if="_loginok">
				<h2>欢迎回来 {{ _userid }}</h2>
				<router-link to="/web" role="button" class="button">进入 BMYBBS</router-link>
			</div>
			<div class="form-container" v-else>
				<h2>欢迎访问 BMYBBS</h2>
				<div class="input-field">
					<fa icon="user" type="fas"></fa>
					<input type="text" placeholder="账号" v-model="username">
				</div>
				<div class="input-field">
					<fa icon="lock" type="fas"></fa>
					<input type="password" placeholder="密码" v-model="password">
				</div>
				<button @click="login">登录</button>
			</div>
		</div>
	</div>
</template>

<script>
import { BMYLogin } from "@/lib/BMYLogin.js"

export default {
	data() {
		return {
			username: "",
			password: "",
		}
	},
	methods: {
		login() {
			BMYLogin(this.username, this.password, this.$toast, () => {
				this.$router.push("/web");
			});
		},
	},
	props: {
		_userid: String,
		_loginok: Boolean,
		_checked: Boolean,
	}
}
</script>

<style scoped>
.bg-image {
	height: 100vh;
	background: rgb(238, 174, 202);
	background: linear-gradient(180deg, rgba(238,174,202,1) 0%, rgba(148,187,233,1) 100%);
}

.bmy-container {
	position: absolute;
	top: 25%;
	width: 100%;
}

.form-container {
	display: flex;
	align-items: center;
	justify-content: center;
	flex-direction: column;
}

h2 {
	font-size: 2.2rem;
	color: #444;
	margin-bottom: 10px;
}

.input-field {
	max-width: 380px;
	width: 100%;
	height: 55px;
	background-color: #e0e0e0;
	margin: 10px;
	border-radius: 55px;
	display: grid;
	grid-template-columns: 15% 85%;
	padding: 0 .4rem;
}

.input-field svg {
	color: #888;
	height: 55px;
	font-size: 1.1rem;
	margin-left: 20px;
}

.input-field input {
	outline: none;
	border: none;
	background: none;
	line-height: 1;
	font-weight: 600;
	font-size: 1.1rem;
	color: #333;
}

.input-field input::placeholder {
	color: #888;
	font-weight: 500;
}

a.button {
	display: block;
	line-height: 49px;
	text-align: center;
	text-decoration: none;
}

button, a.button {
	width: 150px;
	height: 49px;
	border: none;
	outline: none;
	border-radius:49px;
	background-color: #5995fd;
	color: #fff;
	font-weight: 600;
	margin: 10px 0;
	transition: .5s;
}

button:hover, a.button:hover {
	background-color: #4d84e2;
}
</style>

