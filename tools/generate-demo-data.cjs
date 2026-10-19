const fs = require('node:fs');
const path = require('node:path');

const output = path.resolve(__dirname, '../examples/synthetic-reservoir');
fs.mkdirSync(path.join(output, 'wells'), { recursive: true });
const nx = 50;
const ny = 40;
const nz = 20;
const dx = 40;
const dy = 40;
const x0 = 500000;
const y0 = 3300000;
function top(i, j) {
    return 1700 + 55 * Math.sin(Math.PI * i / nx) * Math.sin(Math.PI * j / ny) + 0.4 * i;
}
function keyword(name, values) {
    const lines = [name];
    for (let index = 0; index < values.length; index += 8) {
        lines.push(values.slice(index, index + 8).join(' '));
    }
    lines.push('/', '');
    return lines.join('\n');
}
const coord = [];
for (let j = 0; j <= ny; ++j) {
    for (let i = 0; i <= nx; ++i) {
        coord.push(x0 + i * dx, y0 + j * dy, 1500, x0 + i * dx, y0 + j * dy, 2100);
    }
}
const zcorn = [];
for (let k = 0; k < nz; ++k) {
    for (let face = 0; face < 2; ++face) {
        for (let j = 0; j < ny; ++j) {
            for (let row = 0; row < 2; ++row) {
                for (let i = 0; i < nx; ++i) {
                    for (let column = 0; column < 2; ++column) {
                        zcorn.push((top(i + column, j + row) + (k + face) * 10).toFixed(5));
                    }
                }
            }
        }
    }
}
const fields = { ACTNUM: [], PORO: [], PERMX: [], PERMY: [], PERMZ: [], SATNUM: [] };
let active = 0;
for (let k = 0; k < nz; ++k) {
    for (let j = 0; j < ny; ++j) {
        for (let i = 0; i < nx; ++i) {
            const on = !(i < 3 && j < 4) && !(i >= 47 && j >= 36);
            const channel = Math.exp(-Math.pow((j - 18 - 6 * Math.sin(i / 9)) / 5, 2));
            const poro = 0.12 + 0.09 * channel + 0.025 * Math.cos(k / 3);
            const perm = 15 + 700 * channel * Math.exp(-k / 28);
            fields.ACTNUM.push(on ? 1 : 0);
            active += on ? 1 : 0;
            fields.PORO.push(poro.toFixed(6));
            fields.PERMX.push(perm.toFixed(4));
            fields.PERMY.push((perm * 0.65).toFixed(4));
            fields.PERMZ.push((perm * 0.06).toFixed(4));
            fields.SATNUM.push(k < 7 ? 1 : k < 14 ? 2 : 3);
        }
    }
}
let deck = '-- Synthetic demonstration data; not a simulation-ready case.\n';
deck += keyword('SPECGRID', [nx, ny, nz, 1, 'F']);
deck += keyword('COORD', coord) + keyword('ZCORN', zcorn);
for (const [name, values] of Object.entries(fields)) {
    deck += keyword(name, values);
}
fs.writeFileSync(path.join(output, 'SYNTHETIC.GRDECL'), deck);
const definitions = [
    ['PROD_V1', 12, 12, 0, 0, 0],
    ['INJ_V1', 38, 29, 0, 0, 0],
    ['PROD_D1', 15, 25, 260, -180, 1],
    ['INJ_D1', 35, 12, -220, 220, 1],
    ['PROD_H1', 9, 19, 1100, 100, 2],
    ['PROD_H2', 39, 26, -900, -180, 2]
];
const headers = ['VERSION 1', 'BEGIN HEADER', '1, Name', '2, Surface X', '3, Surface Y',
    '4, Well symbol', '5, Well datum name', '6, Well datum value', '7, TD (MD)', '8, Description', 'END HEADER'];
const manifest = { synthetic: true, dimensions: [nx, ny, nz], cells: nx * ny * nz, activeCells: active,
    units: { length: 'm', permeability: 'mD', porosity: 'fraction' },
    coordinateSystem: 'Local Cartesian; X east, Y north, Z depth positive down; no assigned CRS', wells: [] };
for (const [name, i, j, offsetX, offsetY, type] of definitions) {
    const headX = x0 + (i + 0.5) * dx;
    const headY = y0 + (j + 0.5) * dy;
    const rows = [`# WELL NAME: ${name}`, `# WELL HEAD X-COORDINATE: ${headX}`,
        `# WELL HEAD Y-COORDINATE: ${headY}`, '# WELL KB: 0',
        '# SYNTHETIC DATA: X east, Y north, Z and TVD positive down in metres.',
        '# MD AND TVD ARE REFERENCED (=0) AT KB AND INCREASE DOWNWARDS',
        'MD X Y Z TVD DX DY AZIM INCL'];
    let previous = null;
    let md = 0;
    let hits = 0;
    for (let station = 0; station <= 120; ++station) {
        const fraction = station / 120;
        const bend = Math.max(0, (fraction - 0.65) / 0.35);
        const lateral = type === 2 ? bend * bend : type === 1 ? fraction * fraction : 0;
        const x = headX + offsetX * lateral;
        const y = headY + offsetY * lateral;
        const z = type === 2 ? 1840 * Math.sin(fraction * Math.PI / 2) : 2000 * fraction;
        let inclination = 0;
        let azimuth = 0;
        if (previous) {
            const vx = x - previous.x;
            const vy = y - previous.y;
            const vz = z - previous.z;
            md += Math.hypot(vx, vy, vz);
            inclination = Math.atan2(Math.hypot(vx, vy), vz) * 180 / Math.PI;
            azimuth = (Math.atan2(vx, vy) * 180 / Math.PI + 360) % 360;
        }
        const ci = Math.floor((x - x0) / dx);
        const cj = Math.floor((y - y0) / dy);
        const roof = top((x - x0) / dx, (y - y0) / dy);
        if (ci >= 0 && ci < nx && cj >= 0 && cj < ny && z >= roof && z <= roof + nz * 10) {
            ++hits;
        }
        rows.push([md, x, y, z, z, x - headX, y - headY, azimuth, inclination].map(value => value.toFixed(6)).join(' '));
        previous = { x, y, z };
    }
    if (hits === 0) {
        throw new Error(`${name} does not intersect the reservoir`);
    }
    fs.writeFileSync(path.join(output, 'wells', `${name}.dev`), rows.join('\n') + '\n');
    headers.push(`"${name}" ${headX} ${headY} 1 "KB" 0 ${md.toFixed(6)} "Synthetic demonstration well"`);
    manifest.wells.push({ name, trajectory: `wells/${name}.dev`, stations: 121, measuredDepth: Number(md.toFixed(6)), reservoirStations: hits });
}
fs.writeFileSync(path.join(output, 'Wellheader.txt'), headers.join('\n') + '\n');
fs.writeFileSync(path.join(output, 'manifest.json'), JSON.stringify(manifest, null, 2) + '\n');
console.log(JSON.stringify(manifest, null, 2));
