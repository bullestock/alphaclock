window.addEventListener('load', () => {
    'use strict'

    let byId = id => document.getElementById(id)
    let ws = new WebSocket('ws://' + window.location.host + '/ws')
    ws.binaryType = 'arraybuffer'

    let manBtn = byId('manual')
    let normBtn = byId('normal')
    let fastBtn = byId('fast')

    let discBtn = byId('discrete')
    let contBtn = byId('continuous')

    let hoursUpBtn = byId('hoursUp')
    let hoursDownBtn = byId('hoursDown')
    let minsUpBtn = byId('minsUp')
    let minsDownBtn = byId('minsDown')
    let secsUpBtn = byId('secsUp')
    let secsDownBtn = byId('secsDown')
    let hoursUpFastBtn = byId('hoursUpFast')
    let hoursDownFastBtn = byId('hoursDownFast')
    let minsUpFastBtn = byId('minsUpFast')
    let minsDownFastBtn = byId('minsDownFast')
    let secsUpFastBtn = byId('secsUpFast')
    let secsDownFastBtn = byId('secsDownFast')

    let hoursSetZeroBtn = byId('hoursSetZero')
    let minsSetZeroBtn = byId('minsSetZero')
    let secsSetZeroBtn = byId('secsSetZero')
    let hoursGoToZeroBtn = byId('hoursGoToZero')
    let minsGoToZeroBtn = byId('minsGoToZero')
    let secsGoToZeroBtn = byId('secsGoToZero')
    
    ws.addEventListener('message', e => {
        console.log('WS msg: ' + e)
    })

    // Add event handlers for buttons
    const upButtons = {
        0: hoursUpBtn,
        1: minsUpBtn,
        2: secsUpBtn,
        10: hoursUpFastBtn,
        11: minsUpFastBtn,
        12: secsUpFastBtn
    }
    const downButtons = {
        0: hoursDownBtn,
        1: minsDownBtn,
        2: secsDownBtn,
        10: hoursDownFastBtn,
        11: minsDownFastBtn,
        12: secsDownFastBtn
    }
    const identifiers = {
        0: 'h',
        1: 'm',
        2: 's'
    }

    function handleModeClick(mode) {
        let dview = new DataView(new ArrayBuffer(2))
        dview.setUint8(0, 1)
        dview.setUint8(1, Number(mode))
        ws.send(dview.buffer)
    }

    function handleHourModeClick(mode) {
        let dview = new DataView(new ArrayBuffer(2))
        dview.setUint8(0, 2)
        dview.setUint8(1, Number(mode))
        ws.send(dview.buffer)
    }

    function handleClick(is_up_button, is_mouse_down, is_fast, ident) {
        console.log(identifiers[ident] + ' ' +
                    (is_up_button ? 'up' : 'down ') + ': ' + 
                    (is_mouse_down ? 'start' : 'stop'))
        let dview = new DataView(new ArrayBuffer(2))
        dview.setUint8(0, 0)
        let arg = Number(ident)
        if (is_up_button)
            arg |= 128
        if (is_mouse_down)
            arg |= 64
        if (is_fast)
            arg |= 32
        dview.setUint8(1, arg)
        ws.send(dview.buffer)
    }

    function handleSetZeroClick(hand) {
        let dview = new DataView(new ArrayBuffer(2))
        dview.setUint8(0, 3)
        dview.setUint8(1, Number(hand))
        ws.send(dview.buffer)
    }

    function handleGoToZeroClick(hand) {
        let dview = new DataView(new ArrayBuffer(2))
        dview.setUint8(0, 4)
        dview.setUint8(1, Number(hand))
        ws.send(dview.buffer)
    }

    manBtn.addEventListener('pointerdown', () => {
        handleModeClick(0)
    })
    normBtn.addEventListener('pointerdown', () => {
        handleModeClick(1)
    })
    fastBtn.addEventListener('pointerdown', () => {
        handleModeClick(2)
    })

    discBtn.addEventListener('pointerdown', () => {
        handleHourModeClick(0)
    })
    contBtn.addEventListener('pointerdown', () => {
        handleHourModeClick(1)
    })
    
    for (const ident in upButtons) {
        if (upButtons.hasOwnProperty(ident)) {
            const upBtn = upButtons[ident]
            let kind = ident
            let fast = false
            if (ident >= 10) {
                kind -= 10
                fast = true
            }
            upBtn.addEventListener('pointerdown', () => {
                handleClick(true, true, fast, kind)
            })
            upBtn.addEventListener('pointerup', () => {
                handleClick(true, false, fast, kind)
            })
            upBtn.addEventListener('pointerout', () => {
                handleClick(true, false, fast, kind)
            })
            const downBtn = downButtons[ident]
            downBtn.addEventListener('pointerdown', () => {
                handleClick(false, true, fast, kind)
            })
            downBtn.addEventListener('pointerup', () => {
                handleClick(false, false, fast, kind)
            })
            downBtn.addEventListener('pointerout', () => {
                handleClick(false, false, fast, kind)
            })
        }
    }

    hoursSetZeroBtn.addEventListener('pointerdown', () => {
        handleSetZeroClick(0)
    })
    minsSetZeroBtn.addEventListener('pointerdown', () => {
        handleSetZeroClick(1)
    })
    secsSetZeroBtn.addEventListener('pointerdown', () => {
        handleSetZeroClick(2)
    })

    hoursGoToZeroBtn.addEventListener('pointerdown', () => {
        handleGoToZeroClick(0)
    })
    minsGoToZeroBtn.addEventListener('pointerdown', () => {
        handleGoToZeroClick(1)
    })
    secsGoToZeroBtn.addEventListener('pointerdown', () => {
        handleGoToZeroClick(2)
    })
})

