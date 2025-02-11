import Fastify from 'fastify';
const fastify = Fastify();
import HTMLParser from 'node-html-parser';
import fs from 'fs';

// Read HTML content
const box = fs.readFileSync('./index.html', 'utf-8');

fastify.get('/', (req, res) => {
    const note = req.query.note;
    const trustedDomains = [];
    if (note) {
        const parsed = HTMLParser.parse(note);
        const elements = parsed.getElementsByTagName('*');
        for (let el of elements) {
            const src = el.getAttribute('src');
            if (src) {
                trustedDomains.push(src);
            }
        }
    }

    // Base CSP - participants need to inject their own report-uri
    const csp = [
        "default-src 'none'",
        "style-src 'unsafe-inline'",
        "script-src 'unsafe-inline'"
    ];

    if (trustedDomains.length) {
        csp.push(`img-src ${trustedDomains.join(' ')}`);
    }

    res.header('Content-Security-Policy', csp.join('; '));
    res.type('text/html');
    return res.send(box);
});

// Admin bot endpoint
fastify.post('/submit', async (req, reply) => {
    const { url } = req.body;
    if (!url.match(/^https?:\/\/[^\s/$.?#].[^\s]*$/)) {
        return reply.status(400).send({ error: 'Invalid URL' });
    }
    return { success: true, message: 'Admin bot will visit your URL soon' };
});

fastify.listen({ host: '0.0.0.0', port: 8080 });


