[![npm](https://img.shields.io/npm/v/autolayout.wasm.svg?style=flat-square)](https://www.npmjs.com/package/autolayout.wasm)

# autolayout.wasm
This library is inspired by [autolayout.js](https://github.com/IjzerenHein/autolayout.js), but is several times faster.

# install
```bash 
npm i -S autolayout.wasm
```

# basic usage
```typescript

import AutoLayout from "autolayout.wasm";

//we need to wait the .wasm file loaded
AutoLayout.ready.then(()=>
{
    let view =  new AutoLayout.View();
    let cons = AutoLayout.parse_evfl("H:|-[a]-[b]-|");
    view.raw_addConstraints(cons, false);
    
    view.update();
    
    let subViews = {};
    view.getSubViews(subViews);
    
    for(let name in subViews)
    {
        let sv = subViews[name];
        console.log(`${sv.left()}, ${sv.top()}, ${sv.width()}, ${sv.height()}`);
    }
});
	
```

# todo: documentation