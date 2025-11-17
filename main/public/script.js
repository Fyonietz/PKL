// script.js - vanilla JS version of CompanyDetail
// Data (mock)
const MOCK_COMPANIES = [
  {
    id: "1",
    name: "PT. Solusi Teknologi",
    bidang: "IT & Software",
    jurusan: ["Teknik Informatika", "Sistem Informasi"],
    benefit: ["Sertifikat", "Bimbingan Mentor", "Akses ke tool berbayar"],
    alamat: "Jalan Merdeka No. 10",
    deskripsi:
      "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s.",
    kuota: 5,
    syarat: [
      "Mengumpulkan CV",
      "Surat pengantar dari sekolah",
      "Tidak memiliki catatan pelanggaran",
    ],
    gambar: "https://via.placeholder.com/800x400",
    phone: "+628123456789",
    email: "info@solusiteknologi.id",
  },
  {
    id: "2",
    name: "CV. Kreatif Media",
    bidang: "Media & Desain",
    jurusan: ["Desain Grafis", "Multimedia"],
    benefit: ["Portofolio proyek nyata", "Sertifikat"],
    alamat: "Jl. Sudirman No. 45",
    deskripsi: "",
    kuota: 3,
    syarat: [
      "Portofolio (opsional)",
      "Minimal rata-rata 70",
      "Wajib memakai pakaian rapi saat interview",
    ],
    gambar: "https://via.placeholder.com/800x400",
    phone: "+6281266591758",
    email: "contact@kreatifmedia.co",
  },
  {
    id: "3",
    name: "PT. Sentra Industri",
    bidang: "Manufaktur",
    jurusan: ["Teknik Mesin", "Teknik Elektro"],
    benefit: ["Pengalaman lapangan", "SK Praktik"],
    alamat: "Kawasan Industri Blok B",
    deskripsi: "Pabrik manufaktur komponen elektronik.",
    kuota: 9,
    syarat: [
      "Penggunaan alat keselamatan saat praktik",
      "Mengikuti sesi orientasi pabrik",
    ],
    gambar: "https://via.placeholder.com/800x400",
    phone: "+628111222333",
    email: "hr@sentraindustri.com",
  },
  {
    id: "4",
    name: "Yayasan Pendidikan ABC",
    bidang: "Pendidikan",
    jurusan: ["Pendidikan", "Administrasi"],
    benefit: ["Sertifikat", "Pelatihan Softskill"],
    alamat: "Jl. Pendidikan No. 3",
    deskripsi: "Lembaga pendidikan yang berfokus pada pelatihan vokasi.",
    kuota: 4,
    syarat: ["Surat izin orangtua/wali", "Mengikuti wawancara singkat"],
    gambar: "https://via.placeholder.com/800x400",
    phone: "+6289988776655",
    email: "info@yayasanabc.org",
  },
  {
    id: "5",
    name: "PT. Green Agro",
    bidang: "Pertanian",
    jurusan: ["Agribisnis", "Teknik Pertanian"],
    benefit: ["Praktik Lapangan", "Sertifikat"],
    alamat: "Desa Makmur, Kec. Sejahtera",
    deskripsi: "Perusahaan agrikultur dan pertanian berkelanjutan.",
    kuota: 8,
    syarat: [
      "Pakaian lapangan sesuai ketentuan",
      "Ketersediaan transportasi ke lokasi kerja",
    ],
    gambar: "https://via.placeholder.com/800x400",
    phone: "+628777666555",
    email: "hello@greenagro.id",
  },
];

const KEY = "selectedCompany";

function $(sel, ctx = document) {
  return ctx.querySelector(sel);
}
function $all(sel, ctx = document) {
  return Array.from(ctx.querySelectorAll(sel));
}

function getCurrentCompany() {
  const params = new URLSearchParams(location.search);
  const id = params.get("id") || MOCK_COMPANIES[0].id;
  return MOCK_COMPANIES.find((c) => c.id === id) || MOCK_COMPANIES[0];
}

function loadSelectedPayload() {
  try {
    const s = localStorage.getItem(KEY);
    if (!s) return null;
    return JSON.parse(s);
  } catch (e) {
    return null;
  }
}

function saveSelectedPayload(payload) {
  try {
    localStorage.setItem(KEY, JSON.stringify(payload));
    window.dispatchEvent(new Event("selectedCompanyChanged"));
  } catch (e) {
    console.error(e);
  }
}

function removeSelectedPayloadIfMatches(id) {
  try {
    const s = loadSelectedPayload();
    if (s && s.id === id) {
      localStorage.removeItem(KEY);
      window.dispatchEvent(new Event("selectedCompanyChanged"));
    }
  } catch (e) {
    console.error(e);
  }
}

function render() {
  const app = document.getElementById("app");
  const company = getCurrentCompany();
  const selectedPayload = loadSelectedPayload();
  const isPlaceSelected = selectedPayload && selectedPayload.id === company.id;

  app.innerHTML = `
    <header class="bg-white shadow-sm border-b border-gray-100 mb-6">
      <div class="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 py-4">
        <div class="flex items-center justify-between"></div>
      </div>
    </header>

    <main>
      <button id="backBtn" class="flex items-center space-x-2 text-gray-600 hover:text-gray-900 mb-6 transition-colors group">
        <i data-lucide="arrow-left" class="w-5 h-5 group-hover:-translate-x-1 transition-transform"></i>
        <span class="font-medium">Kembali</span>
      </button>

      <div class="grid grid-cols-1 lg:grid-cols-3 gap-8">
        <div class="lg:col-span-2 space-y-6">
          <div class="bg-white rounded-2xl shadow-lg p-8 border border-gray-100">
            <div class="flex items-start justify-between mb-6">
              <div class="flex items-center space-x-4">
                <div class="w-20 h-20 bg-gradient-to-br from-blue-500 to-purple-600 rounded-2xl flex items-center justify-center shadow-lg">
                  <span class="text-white text-2xl font-bold">Logo</span>
                </div>
                <div>
                  <h1 class="text-3xl font-bold text-gray-900 mb-1">${
                    company.name
                  }</h1>
                  <div class="flex items-center space-x-4 text-sm text-gray-600">
                    <span class="flex items-center space-x-1">
                      <i data-lucide="building-2" class="w-4 h-4"></i>
                      <span>${company.bidang}</span>
                    </span>
                    <span class="flex items-center space-x-1">
                      <i data-lucide="star" class="w-4 h-4 text-yellow-500"></i>
                      <span>4.8 (124 ulasan)</span>
                    </span>
                  </div>
                </div>
              </div>
              <div class="flex items-center space-x-2 bg-green-100 px-3 py-1 rounded-full">
                <div class="w-2 h-2 bg-green-500 rounded-full animate-pulse"></div>
                <span class="text-sm font-medium text-green-700">Aktif</span>
              </div>
            </div>

            <div class="grid grid-cols-2 gap-4 mb-6">
              <div class="flex items-center space-x-3 p-3 bg-gray-50 rounded-lg">
                <i data-lucide="map-pin" class="w-5 h-5 text-gray-600"></i>
                <div>
                  <p class="text-xs text-gray-500">Lokasi</p>
                  <p class="text-sm font-medium text-gray-900">${
                    company.alamat
                  }</p>
                </div>
              </div>
              <div class="flex flex-col space-y-2">
                <div class="flex items-center space-x-3 p-3 bg-gray-50 rounded-lg">
                  <i data-lucide="users" class="w-5 h-5 text-gray-600"></i>
                  <div>
                    <p class="text-xs text-gray-500">Ukuran Perusahaan</p>
                    <p class="text-sm font-medium text-gray-900">${
                      company.kuota
                        ? Math.min(50, company.kuota * 10) + " Karyawan"
                        : "—"
                    }</p>
                  </div>
                </div>

                <div class="flex items-center space-x-3 p-3 bg-gray-50 rounded-lg">
                  <i data-lucide="phone" class="w-5 h-5 text-gray-600"></i>
                  <div>
                    <p class="text-xs text-gray-500">Kontak</p>
                    <p class="text-sm font-medium text-gray-900">
                      ${
                        company.phone
                          ? `<a href="tel:${company.phone}" class="text-blue-600 hover:underline">${company.phone}</a>`
                          : "—"
                      }
                    </p>
                    ${
                      company.email
                        ? `<p class="text-sm text-gray-500"><a href="mailto:${company.email}" class="hover:underline">${company.email}</a></p>`
                        : ""
                    }
                  </div>
                </div>
              </div>
            </div>
          </div>

          <div class="bg-white rounded-2xl shadow-lg p-8 border border-gray-100">
            <h2 class="text-xl font-bold text-gray-900 mb-4 flex items-center">
              <div class="w-8 h-8 bg-blue-100 rounded-lg flex items-center justify-center mr-3">
                <i data-lucide="briefcase" class="w-5 h-5 text-blue-600"></i>
              </div>
              Tentang Perusahaan
            </h2>
            <div class="space-y-4 text-gray-700">
              <p>${
                company.deskripsi || "Deskripsi perusahaan belum tersedia."
              }</p>
              <p>${
                company.bidang
                  ? `Perusahaan bergerak di bidang ${company.bidang}.`
                  : "Informasi bidang belum tersedia."
              }</p>
            </div>
          </div>

          <div class="bg-white rounded-2xl shadow-lg p-8 border border-gray-100">
            <h2 class="text-xl font-bold text-gray-900 mb-4 flex items-center">
              <div class="w-8 h-8 bg-yellow-100 rounded-lg flex items-center justify-center mr-3">
                <i data-lucide="clock" class="w-5 h-5 text-yellow-600"></i>
              </div>
              Syarat Pendaftaran
            </h2>
            <div class="space-y-3 text-gray-700">
              ${
                company.syarat && company.syarat.length
                  ? `<ul class="list-inside list-disc space-y-2">${company.syarat
                      .map((s) => `<li class="text-sm text-gray-700">${s}</li>`)
                      .join("")}</ul>`
                  : `<div class="text-sm text-gray-500">Belum ada syarat tercatat.</div>`
              }
            </div>
          </div>

          <div class="bg-white rounded-2xl shadow-lg p-8 border border-gray-100">
            <h2 class="text-xl font-bold text-gray-900 mb-4 flex items-center">
              <div class="w-8 h-8 bg-purple-100 rounded-lg flex items-center justify-center mr-3">
                <i data-lucide="palette" class="w-5 h-5 text-purple-600"></i>
              </div>
              Jurusan yang Cocok
            </h2>
            <div class="grid grid-cols-1 sm:grid-cols-2 gap-4">
              ${
                company.jurusan && company.jurusan.length
                  ? company.jurusan
                      .map(
                        (j) => `
                <div class="flex items-center space-x-3 p-4 bg-gradient-to-r from-blue-50 to-purple-50 rounded-xl border border-blue-100 hover:shadow-md transition-shadow">
                  <div class="w-12 h-12 bg-white rounded-lg flex items-center justify-center shadow-sm">
                    <i data-lucide="palette" class="w-6 h-6 text-blue-600"></i>
                  </div>
                  <div>
                    <h3 class="font-semibold text-gray-900">${j}</h3>
                    <p class="text-sm text-gray-600">${j}</p>
                  </div>
                </div>
              `
                      )
                      .join("")
                  : `<div class="text-sm text-gray-500">Tidak ada jurusan tercatat.</div>`
              }
            </div>
          </div>
        </div>

        <div class="space-y-6">
          <div class="bg-white rounded-2xl shadow-lg p-6 border border-gray-100 sticky top-6">
            <h2 class="text-xl font-bold text-gray-900 mb-4 flex items-center">
              <div class="w-8 h-8 bg-green-100 rounded-lg flex items-center justify-center mr-3">
                <i data-lucide="award" class="w-5 h-5 text-green-600"></i>
              </div>
              Benefit & Fasilitas
            </h2>
            <div class="space-y-3">
              ${
                company.benefit && company.benefit.length
                  ? company.benefit
                      .map(
                        (b) => `
                <div class="flex items-start space-x-3 p-3 bg-green-50 rounded-lg">
                  <i data-lucide="check-circle" class="w-5 h-5 text-green-600 mt-0.5 flex-shrink-0"></i>
                  <div>
                    <p class="font-medium text-gray-900">${b}</p>
                    <p class="text-sm text-gray-600">&nbsp;</p>
                  </div>
                </div>
              `
                      )
                      .join("")
                  : `<div class="text-sm text-gray-500">Tidak ada benefit tercatat.</div>`
              }

              <div class="mt-6 pt-6 border-t border-gray-200">
                <div class="flex items-center justify-between mb-4">
                  <span class="text-sm text-gray-600">Durasi Magang</span>
                  <span class="text-sm font-semibold text-gray-900">3-6 Bulan</span>
                </div>
                <div class="flex items-center justify-between mb-4">
                  <span class="text-sm text-gray-600">Slot Tersedia</span>
                  <span class="text-sm font-semibold ${
                    isPlaceSelected ? "text-orange-600" : "text-green-600"
                  }">${
    typeof company.kuota === "number" ? company.kuota + " Slot" : "—"
  }</span>
                </div>
                <div class="flex items-center justify-between">
                  <span class="text-sm text-gray-600">Deadline Pendaftaran</span>
                  <span class="text-sm font-semibold text-gray-900">30 Nov 2024</span>
                </div>
              </div>

              <div class="mt-6 space-y-3">
                ${
                  !isPlaceSelected
                    ? selectedPayload && selectedPayload.id !== company.id
                      ? `<button disabled class="w-full bg-gray-200 text-gray-500 font-semibold py-3 px-6 rounded-xl cursor-not-allowed flex items-center justify-center space-x-2">Anda sudah memilih ${
                          selectedPayload.name || "tempat lain"
                        }</button>`
                      : `<button id="selectPlaceBtn" class="w-full bg-gradient-to-r from-blue-600 to-purple-600 text-white font-semibold py-3 px-6 rounded-xl hover:shadow-lg transform hover:-translate-y-0.5 transition-all duration-200 flex items-center justify-center space-x-2">
                    <span>Pilih Tempat Ini</span>
                    <svg class="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24"><path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M13 7l5 5m0 0l-5 5m5-5H6"></path></svg>
                  </button>`
                    : `
                  <div class="space-y-3">
                    <button class="w-full px-4 py-3 bg-indigo-600 text-white rounded-xl hover:bg-indigo-700 transition-colors font-semibold flex items-center justify-center" aria-pressed="true">Tersimpan ✓</button>
                    <button id="backBtn2" class="w-full px-4 py-3 border border-gray-200 rounded-xl text-gray-700 hover:bg-gray-50 transition-colors">Kembali</button>
                    <button id="cancelSelectionBtn" class="w-full px-4 py-3 bg-red-50 text-red-700 border border-red-100 rounded-xl hover:bg-red-100 transition-colors">Batalkan Pilihan</button>
                  </div>
                `
                }
              </div>
            </div>
          </div>
        </div>
      </div>
    </main>

    <!-- Modal -->
    <div id="confirmModal" class="hidden fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50 p-4">
      <div class="bg-white rounded-2xl shadow-xl max-w-md w-full p-6 transform transition-all">
        <div class="flex items-center justify-between mb-4">
          <h3 class="text-xl font-bold text-gray-900">Konfirmasi Pilihan</h3>
          <button id="closeModalBtn" class="text-gray-400 hover:text-gray-600 transition-colors"><i data-lucide="x" class="w-6 h-6"></i></button>
        </div>
        <div class="mb-6">
          <div class="flex items-center justify-center w-16 h-16 bg-yellow-100 rounded-full mx-auto mb-4">
            <i data-lucide="alert-triangle" class="w-8 h-8 text-yellow-600"></i>
          </div>
          <p class="text-center text-gray-700">Apakah Anda yakin ingin memilih tempat magang ini? Pilihan ini akan mengurangi slot yang tersedia.</p>
        </div>
        <div class="flex flex-col sm:flex-row gap-3">
          <button id="modalCancelBtn" class="flex-1 px-4 py-2 bg-gray-200 text-gray-800 rounded-lg hover:bg-gray-300 transition-colors font-medium">Batal</button>
          <button id="modalConfirmBtn" class="flex-1 px-4 py-2 bg-blue-600 text-white rounded-lg hover:bg-blue-700 transition-colors font-medium">Ya, Pilih</button>
        </div>
      </div>
    </div>
  `;

  // after rendering, wire events
  // lucide icons replacement
  if (window.lucide) window.lucide.replace();

  const backBtn = $("#backBtn");
  if (backBtn) backBtn.addEventListener("click", () => window.history.back());
  const backBtn2 = $("#backBtn2");
  if (backBtn2) backBtn2.addEventListener("click", () => window.history.back());

  const selectBtn = $("#selectPlaceBtn");
  const confirmModal = $("#confirmModal");
  const modalCancelBtn = $("#modalCancelBtn");
  const modalConfirmBtn = $("#modalConfirmBtn");
  const closeModalBtn = $("#closeModalBtn");
  const cancelSelectionBtn = $("#cancelSelectionBtn");

  if (selectBtn)
    selectBtn.addEventListener("click", () => {
      if (confirmModal) confirmModal.classList.remove("hidden");
    });
  if (modalCancelBtn)
    modalCancelBtn.addEventListener("click", () => {
      if (confirmModal) confirmModal.classList.add("hidden");
    });
  if (closeModalBtn)
    closeModalBtn.addEventListener("click", () => {
      if (confirmModal) confirmModal.classList.add("hidden");
    });

  if (modalConfirmBtn)
    modalConfirmBtn.addEventListener("click", () => {
      // persist selection
      const payload = {
        id: company.id,
        name: company.name,
        kuota: company.kuota,
      };
      saveSelectedPayload(payload);
      if (confirmModal) confirmModal.classList.add("hidden");
      // re-render to update UI
      render();
    });

  if (cancelSelectionBtn)
    cancelSelectionBtn.addEventListener("click", () => {
      removeSelectedPayloadIfMatches(company.id);
      render();
    });
}

// listen for changes from other windows/components
window.addEventListener("selectedCompanyChanged", () => {
  render();
});

// init
document.addEventListener("DOMContentLoaded", () => {
  render();
});
