-- xkcd (https://xkcd.com): one series, each comic is a chapter (one page).
-- The comics are licensed CC BY-NC 2.5 by Randall Munroe.
SOURCE = {
    id = "xkcd", name = "xkcd", version = "2.0", kind = "manga",
    site = "https://xkcd.com",
}

local function get(url, cache_seconds)
    local body, status = http.get(url, nil, cache_seconds)
    if not body then error("network error: " .. tostring(status), 0) end
    return body, status
end

function search(query, page)
    if page > 1 then return {} end
    -- The latest comic changes: cached 1 hour
    local latest = json.decode(get("https://xkcd.com/info.0.json", 3600))
    if query ~= "" and not ("xkcd " .. latest.title):lower():find(query:lower(), 1, true) then
        return {}
    end
    return { {
        id = "xkcd", title = "xkcd", url = "https://xkcd.com",
        cover = latest.img, status = "publishing", year = "2005",
        description = "A webcomic of romance, sarcasm, math, and language. " .. latest.num ..
                      " comics, latest: #" .. latest.num .. " " .. latest.title,
    } }
end

-- Every comic from the archive page (number, title, date), oldest first
function chapters(manga)
    local doc = html.parse(get("https://xkcd.com/archive/", 3600))
    local list = {}
    for _, a in ipairs(doc:select("#middleContainer a[href]")) do
        local num = a:attr("href"):match("^/(%d+)/$")
        if num then
            table.insert(list, 1, {
                id = num, title = "#" .. num .. " " .. a:text(),
                number = num, date = a:attr("title") or "",
            })
        end
    end
    return list
end

function pages(chapter)
    local body, status = get("https://xkcd.com/" .. chapter.id .. "/info.0.json", 31536000)
    if status ~= 200 then error("comic #" .. chapter.id .. ": HTTP " .. status, 0) end
    local comic = json.decode(body)
    if not comic.img or comic.img == "" or comic.img:match("/$") then error("comic #" .. chapter.id .. " has no image (interactive comic)", 0) end
    return { comic.img }
end
