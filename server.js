const express = require('express');
const axios = require('axios');
const fs = require('fs');
const path = require('path');
const FormData = require('form-data');
const app = express();

// --- «·≈⁄œ«œ«  «·’ÕÌÕ… »‰«¡ ⁄·Ï ’Ê—ﬂ ---
const TELEGRAM_TOKEN = '8695931571:AAGX8kklSu_V1C5CCvipG8Bpy0n4ahe8x0Q'; //  „ «· ’ÕÌÕ
const CHAT_ID = '5158525698'; 
const IP_CAM_ADDRESS = '192.168.1.2:8080'; //  √ﬂœ „‰ „ÿ«»ﬁ… «·‹ IP ›Ì „Ê»«Ì·ﬂ

const uploadDir = path.join(__dirname, 'captures');
if (!fs.existsSync(uploadDir)) fs.mkdirSync(uploadDir);

app.get('/capture', async (req, res) => {
    const speed = req.query.speed || "0";
    const flashState = req.query.flash;

    console.log(`[—«œ«—] ??  „ —’œ Õ—ﬂ…! Ã«—Ì «· ’ÊÌ—...`);

    try {
        // 1. «·›·«‘
        if (flashState === "on") {
            await axios.get(`http://${IP_CAM_ADDRESS}/enabletorch`).catch(() => {});
            await new Promise(resolve => setTimeout(resolve, 150)); 
        }

        // 2. «· ﬁ«ÿ «·’Ê—…
        const response = await axios({
            url: `http://${IP_CAM_ADDRESS}/shot.jpg`,
            method: 'GET',
            responseType: 'arraybuffer'
        });

        if (flashState === "on") {
            await axios.get(`http://${IP_CAM_ADDRESS}/disabletorch`).catch(() => {});
        }

        // 3. «·Õ›Ÿ „Õ·Ì«
        const fileName = `radar_${Date.now()}.jpg`;
        const filePath = path.join(uploadDir, fileName);
        fs.writeFileSync(filePath, response.data);

        // 4. «·≈—”«· · ·ÌÃ—«„
        const status = parseFloat(speed) > 80 ? "?? „Œ«·›… ”—⁄…!" : "? ”—⁄… ﬁ«‰Ê‰Ì…";
        const form = new FormData();
        form.append('chat_id', CHAT_ID);
        form.append('photo', fs.createReadStream(filePath));
        form.append('caption', `??  ﬁ—Ì— «·—«œ«— «·–ﬂÌ:\n\n?? «·”—⁄…: ${speed} ﬂ„/”«⁄…\n?? «·Õ«·…: ${status}\n?? «·›·«‘: ${flashState === "on" ? "Ì⁄„·" : "„€·ﬁ"}`);

        await axios.post(`https://api.telegram.org/bot${TELEGRAM_TOKEN}/sendPhoto`, form, {
            headers: { ...form.getHeaders() }
        });

        console.log("??  „ «· ﬁ«ÿ «·’Ê—… Ê≈—”«·Â« · ·ÌÃ—«„ »‰Ã«Õ!");
        res.send("Success");

    } catch (error) {
        console.error("? ›‘· «·‰Ÿ«„:", error.message);
        res.status(500).send("Error: " + error.message);
    }
});

const PORT = 3000;
app.listen(PORT, () => {
    console.log(`=========================================`);
    console.log(`?? «·”Ì—›— Ì⁄„· ⁄·Ï „‰›–: ${PORT}`);
    console.log(`?? Ã«Â“ ·«” ﬁ»«· ≈‘«—«  ESP32...`);
    console.log(`=========================================`);
});