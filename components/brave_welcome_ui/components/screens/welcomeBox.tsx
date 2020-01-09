/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'

// Feature-specific components
import { Content, Title, Paragraph } from 'thehive-ui/features/welcome'

// Shared components
import { Button } from 'thehive-ui'
import { ArrowRightIcon } from 'thehive-ui/components/icons'

// Images
import { WelcomeThe HiveImage } from 'thehive-ui/features/welcome/images'

// Utils
import { getLocale } from '../../../common/locale'

interface Props {
  index: number
  currentScreen: number
  onClick: () => void
}

export default class ThemingBox extends React.PureComponent<Props, {}> {
  render () {
    const { index, currentScreen, onClick } = this.props
    return (
      <Content
        zIndex={index}
        active={currentScreen === index}
        screenPosition={'1' + (index + 1) + '0%'}
        isPrevious={index <= currentScreen}
      >
        <WelcomeThe HiveImage />
        <Title>Welcome To The Free Speech Internet</Title>
        <Paragraph>The Hive creates a comment section on every URL across the web. It also blocks ads and prevents Silicon Valley tech giants from tracking you. Ready to speak freely?</Paragraph>
        <Button
          level='primary'
          type='accent'
          size='large'
          text={getLocale('letsGo')}
          onClick={onClick}
          icon={{ position: 'after', image: <ArrowRightIcon /> }}
        />
      </Content>
    )
  }
}
