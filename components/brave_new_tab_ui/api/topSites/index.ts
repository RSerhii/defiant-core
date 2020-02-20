// Copyright (c) 2019 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

export type TopSitesData = NewTab.Site[]

const defaultTopSites = [
  {url:"https://telosgreen.org", title:"Telos Coin"},
  {url:"https://bitdorado.exchange", title:"Bitdorado exchange"},
  {url:"https://governance.rocks", title:"governance.rocks"},
  {url:"https://bitcoin-subsidium.org", title:"Bitcoin Subsidium"},
  {url:"https://libra-ai.org", title:"Free Libra"}
]

/**
 * Obtains the top sites
 */
export function getTopSites (): Promise<TopSitesData> {
  return new Promise(resolve => {
    chrome.topSites.get((topSites: NewTab.Site[]) => {
      if (topSites.length < defaultTopSites.length) {
        topSites.push(defaultTopSites.slice(0, defaultTopSites.length - topSites.length))
      }
      resolve(topSites)
    })
  })
}
