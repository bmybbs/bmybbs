/**
 * 生成回复全文
 * @param text   正文
 * @param author 原文作者
 * @param origin 原文
 * @return 全文
 */
const generateContent = (text, author, origin) => {
	const arr = origin.split("\n"),
		result = [ text, `【 在 ${author} 的大作中提到: 】` ];

	for (let i = 0, j = 0, l = arr.length; i < l && j < 5; i++) {
		if (arr[i] == "")
			continue;

		result.push(`: ${arr[i]}`);
		j++;
	}

	if (result.length == 2) {
		result.push(": ");
	} else {
		result.push(": ...................");
	}

	return result.join("\n");
}

export { generateContent };

