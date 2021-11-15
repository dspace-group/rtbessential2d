import { App, MenuEntry, PageContent, Route } from '@dspace-internal/ui-kit'
import HomeIcon from '@material-ui/icons/Home'
import React from 'react'

const HomePage: React.FC = () => {
  return <div>Home Page</div>
}

const NotFoundPage: React.FC = () => {
  return <div>Page not found</div>
}

const menu: MenuEntry[] = [
  {
    type: 'entry',
    data: {
      type: 'entry',
      data: {
        url: '/',
        label: 'Home',
        icon: <HomeIcon />,
      }
    },
  },
]

const routes: Route[] = [
  {
    path: '/',
    breadcrumbs: [],
  },
]

const MyApp: React.FC = () => {
  return (
    <App menu={menu} routes={routes} notFoundComponent={NotFoundPage}>
      <PageContent />
    </App>
  )
}

export default MyApp;