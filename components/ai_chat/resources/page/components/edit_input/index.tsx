// Copyright (c) 2024 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

import * as React from 'react'

import Button from '@brave/leo/react/button'
import Icon from '@brave/leo/react/icon'

import styles from './style.module.scss'

interface Props {
  text: string
  onSubmit: (text: string) => void
  onCancel: () => void
}

function EditInput(props: Props) {
  const [text, setText] = React.useState(props.text)
  const handleKeyDown = (e: React.KeyboardEvent<HTMLTextAreaElement>) => {
    if (e.key === 'Enter') {
      props.onSubmit(e.currentTarget.value)
    } else if (e.key === 'Escape') {
      props.onCancel()
    }
  }

  return (
    <div className={styles.content}>
      <div className={styles.textareaContainer}>
        <textarea
          value={text}
          onChange={(e) => setText(e.currentTarget.value)}
          onKeyDown={handleKeyDown}
          autoFocus
        />
        <Button
          fab
          kind='plain-faint'
          onClick={props.onCancel}
        >
          <Icon name="close"/>
        </Button>
      </div>
      <div className={styles.actions}>
        <Button
          kind='plain-faint'
          size='small'
          onClick={props.onCancel}
        >
          Cancel
        </Button>
        <Button
          size='small'
          onClick={() => props.onSubmit(text)}
        >
          Save
        </Button>
      </div>
    </div>
  )
}

export default EditInput
