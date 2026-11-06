import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';

// GitHub Pages provides the repository name at build time. The defaults keep
// local `npm run build` usable before a remote repository has been configured.
const repository = process.env.GITHUB_REPOSITORY || 'jamedegnato-kostal/smalltv-ultra-mod';
const owner = repository.split('/')[0];
const repo = repository.split('/')[1] || 'smalltv-settings-firmware';
const site = process.env.SITE || `https://${owner}.github.io`;
const base = process.env.BASE_PATH || (repo.toLowerCase() === `${owner.toLowerCase()}.github.io` ? '/' : `/${repo}`);
const githubUrl = `https://github.com/${repository}`;

export default defineConfig({
  site,
  base,
  integrations: [
    starlight({
      title: 'SmallTV Settings Firmware',
      description:
        'Anleitung und technische Dokumentation zur ESP8266-Firmware für SmallTV und SmallTV Ultra.',
      logo: {
        src: './src/assets/logo.svg',
        replacesTitle: false,
      },
      social: [{ icon: 'github', label: 'GitHub', href: githubUrl }],
      editLink: {
        baseUrl: `${githubUrl}/edit/main/docs/`,
      },
      sidebar: [
        { label: 'Übersicht', link: '/' },
        {
          label: 'Erste Schritte',
          items: [
            { label: 'Gerät einrichten', link: '/getting-started/setup/' },
            { label: 'Firmware installieren', link: '/getting-started/flashing/' },
          ],
        },
        {
          label: 'Bedienung',
          items: [
            { label: 'Schnellstart', link: '/manual/quick-start/' },
            { label: 'Einstellungen', link: '/manual/settings/' },
            { label: 'Fehlerbehebung', link: '/manual/troubleshooting/' },
          ],
        },
        {
          label: 'Themes',
          items: [{ label: 'Themes und Playlist', link: '/features/themes/' }],
        },
        { label: 'Web-API', link: '/api/' },
        {
          label: 'Entwicklung',
          items: [
            { label: 'Firmware bauen', link: '/reference/building/' },
            { label: 'Recovery', link: '/reference/recovery/' },
          ],
        },
      ],
    }),
  ],
});
