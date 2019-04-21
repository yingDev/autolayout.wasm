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
        console.log(`${name}: ${sv.left()}, ${sv.top()}, ${sv.width()}, ${sv.height()}`);
    }
});
	
```

# build
- make sure these are installed:
    - [cmake](https://cmake.org/download/)
    - [emsdk](https://github.com/emscripten-core/emsdk)
    - [boost](https://www.boost.org/users/download/)
- set these environment variables:
    - `EMSCRIPTEN_CMAKE_TOOLCHAIN_FILE` (eg. `/Developer/emsdk/emscripten/1.38.30/cmake/Modules/Platform/Emscripten.cmake`)
    - `BOOST_ROOT` (eg. `/usr/local/Cellar/boost/1.69.0`)
- run `npm test`

# todo: documentation