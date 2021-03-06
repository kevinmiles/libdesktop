#!/bin/env ts-node

import {handlebars} from 'consolidate';
import {readdir,copyFile,mkdir, writeFile} from 'fs/promises';
const jd = require("javadoc");
import { join,resolve } from "path";

const docsDir = resolve(__dirname,"../docs/")
const docsSrcDir = resolve(__dirname,"../src/docs/")


async function copyDir(srcDirPath:string, targetDirPath: string) {
    const files = await readdir(srcDirPath,{withFileTypes:true})
    try {
        await mkdir(targetDirPath,0o755)
    } catch (err) {
        if (err.code != 'EEXIST') {
            throw err
        }
    }
    
    for (const f of files) {
        if (f.isFile()) {
            await copyFile(join(srcDirPath, f.name), join(targetDirPath, f.name))
            console.log(join(srcDirPath, f.name), "--> ", join(targetDirPath, f.name))
            continue
        }
        if (f.isDirectory()) {
            await copyDir(join(srcDirPath, f.name), join(targetDirPath, f.name))
            continue
        }
    }
}

function copyDocsify() {
    const srcDirPath = join(docsSrcDir,"docsify");
    const targetDirPath = join(docsDir,".");
    return copyDir(srcDirPath, targetDirPath)
}

function trimMember(it: Array<string>){
    const res = {
        [it[0].trim().slice(1)]: String((it[1])).slice(0).trim(),
    }      
    return res
}

async function run() {


    await copyDocsify();

    const jsonFile = join(docsDir,"/tmp/dsknapi.json")
    await jd.generate({
        include: [join(__dirname,"../includes/dsknapi.h")],
        exclude: ["**/node_modules/**"],
        format: "json",
        output: jsonFile,
    });
    
    const jsonDocs = require(jsonFile);
    const exports = jsonDocs.success[0].map((block: any) => 
            Object.assign({},...Object.entries(block).map(trimMember as any)));

    const groups = {};
    for (const xp of exports){
        if (xp.group) {
            const group = (groups as any)[xp.group] ||= {
                members: [],
                name: xp.group,
            }
            if (xp.name) {
                group.members.push(xp);
            } else {
                group.descr = xp.descr;
                group.brief = xp.brief;
            }

        }
    }

    const data = {
        groups,
        //exports: exports
        //    .filter((it: { name: string; })=>it.name),
        file: exports
            .filter((it: {name:string})=>!it.name)
            .map((it: {file:string,brief:string,descr:string})=>({
                file:it.file,
                brief:it.brief,
                descr:it.descr
            }))[0],
    };
    
    handlebars(join(__dirname,"../src/docs/cheader.md"), data, async (err, html) => {
        if (err) throw err;
        const target = join(docsDir,"./dsknapi.md")
        await writeFile(target,html)
    });
    console.log(JSON.stringify(data,null,4))
}

run().catch(error => {
    console.error(error)
});