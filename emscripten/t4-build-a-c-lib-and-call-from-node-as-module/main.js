var em_module = require('./a.out.js');

// 需要等待 onRuntimeInitialized
em_module["onRuntimeInitialized"] = function(){
	var result = em_module._add(1,2);
	console.log(result);
}