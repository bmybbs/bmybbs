import { DOMParser, XMLSerializer } from "xmldom"

export const minibmyFormatArticle = content => {
	const doc = new DOMParser().parseFromString(content, "text/html");

	// 处理引言 blockquote
	const bq_arr = [].slice.call(doc.getElementsByTagName("blockquote"));
	bq_arr.forEach(x => {
		x.setAttribute("class", "blockquote")
	});

	// 处理代码块
	const pre_arr = [].slice.call(doc.getElementsByTagName("pre"));
	pre_arr.forEach(x => {
		x.setAttribute("class", "pre");
	});

	return new XMLSerializer().serializeToString(doc);
}

/**
 * 生成回复全文
 * @param text   正文
 * @param author 原文作者
 * @param origin 原文
 * @return 全文
 */
export const generateContent = (text, author, origin) => {
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

